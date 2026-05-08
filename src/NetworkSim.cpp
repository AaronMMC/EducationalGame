#include "NetworkSim.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

// =============================================================================
// PARALLELISM [Phase 6 — Application Development]: NetworkSim.cpp
// =============================================================================
// This file implements the Pipeline parallelism pattern described in the
// Team 3 framework.  The original tick() ran entirely on the main thread.
// Now it is split into two pieces:
//
//   simLoop()  — runs on simThread (background); does the CPU work
//   tick()     — runs on main thread; just reads the latest snapshot
//
// The synchronization boundary is swapSnapshot(), called inside simLoop()
// after every simulated step.  Only that one struct-copy is protected by
// the mutex; everything else in simLoop() is thread-local.
// =============================================================================

void NetworkSim::reset(int avId, bool frozenStart) {
    // PARALLELISM [Phase 6 — Thread Lifecycle]:
    // Always stop the previous sim thread before resetting state.  Failing to
    // join before modifying shared variables would be a data race — exactly
    // the scenario ThreadSanitizer (Phase 7) is configured to detect.
    stop();

    avatarId      = avId;
    progressPct   = 0.f;
    wpm           = 0;
    accuracy      = 95;
    hp            = 100.f;
    multiplier    = 1.f;
    frozen        = frozenStart;   // atomic store — visible to simLoop immediately
    charsTyped    = 0.f;
    virusTimer    = 0.f;
    lastMilestone = 0;
    started       = false;

    // Reset both snapshot buffers so stale data from last round is cleared.
    pending  = SimSnapshot{};
    shadow   = SimSnapshot{};
    sabotageIndices.clear();

    oppSpeed = (35.f + (std::rand() % 30)) / 60.f * 5.f;

    // stopFlag must be cleared AFTER joining the old thread (done in stop())
    // and BEFORE launching a new one, so simLoop() doesn't exit immediately.
    stopFlag  = false;
    connected = false;
}

void NetworkSim::connect() {
    // PARALLELISM [Phase 6 — Thread Launch]:
    // connect() is called from GameScreen::startRound() the moment the player
    // begins typing.  We launch simThread here (not in reset()) to keep the
    // thread's lifetime scoped to the active match, not the lobby/VS screen.
    //
    // std::thread constructor:
    //   std::thread( callable, args... )
    // matches the BNF rule from Phase 4:
    //   <sim-thread> ::= std::thread( &NetworkSim::simLoop, this )
    //
    // textLen is passed as a captured snapshot of targetText.size() at the
    // moment of launch; simLoop treats it as immutable.
    connected = true;
    startTime = std::chrono::steady_clock::now();
    started   = true;

    // NOTE: textLen should ideally be passed here from GameState::targetText.size().
    // For integration, GameScreen::startRound() should call
    //   gs.opponent.connect(gs.targetText.size());
    // and this signature adjusted accordingly.  Shown as 0 here for structure demo.
    simThread = std::thread(&NetworkSim::simLoop, this, 0 /* textLen injected at call site */);
}

void NetworkSim::stop() {
    // PARALLELISM [Phase 6 — Thread Teardown]:
    // 1. Signal the thread to exit via the atomic stop flag.
    // 2. Join — block until simLoop() has returned.
    // This two-step is the standard C++ thread lifecycle idiom.
    // Destroying a std::thread that is still joinable() calls std::terminate(),
    // so joining here (rather than detaching) is essential for correct shutdown.
    if (simThread.joinable()) {
        stopFlag = true;            // atomic store; simLoop() will see this on its next iteration
        simThread.join();           // wait for clean exit — no orphaned thread
    }
}

// =============================================================================
// simLoop() — runs on simThread (BACKGROUND THREAD)
// =============================================================================
// PARALLELISM [Phase 3 — Task Independence]:
// This entire function runs concurrently with the main render loop.  It
// touches ONLY thread-local variables (charsTyped, virusTimer, pending) and
// the shared shadow buffer via the mutex-protected swapSnapshot().
//
// Nothing in this function calls any SFML draw function — SFML's renderer is
// not thread-safe and must remain on the main thread exclusively.
// =============================================================================
void NetworkSim::simLoop(int textLen) {
    using Clock = std::chrono::steady_clock;
    auto lastTick = Clock::now();

    // PARALLELISM [Phase 6 — Stop-flag polling]:
    // The loop checks stopFlag at the top of every iteration.  This is the
    // cooperative cancellation model — the main thread signals, the sim thread
    // responds on its next cycle.  The atomic load is lock-free (no mutex).
    while (!stopFlag.load()) {

        auto now = Clock::now();
        float dt = std::chrono::duration<float>(now - lastTick).count();
        lastTick = now;

        // --- Simulate one step (all local state — no locks needed here) ---

        if (textLen > 0 && !frozen.load()) {   // atomic read of frozen flag
            float jitter   = 1.f + (std::rand() % 30 - 15) / 100.f;
            charsTyped    += oppSpeed * dt * jitter;
            charsTyped     = std::min(charsTyped, (float)textLen);
            pending.progressPct = charsTyped / textLen;

            float elapsed = std::chrono::duration<float>(now - startTime).count() / 60.f;
            pending.wpm   = elapsed > 0.f ? (int)((charsTyped / 5.f) / elapsed) : 0;
            pending.multiplier = std::min(pending.multiplier + 0.01f * dt, 4.f);

            // Damage milestone — queue it into the pending snapshot
            int currentMilestone = (int)charsTyped / 15;
            if (currentMilestone > lastMilestone && charsTyped > 1.f) {
                lastMilestone = currentMilestone;
                pending.damageAccum += 1.f;
            }
        }

        // VIRUS avatar sabotage — runs on sim thread, result shared via snapshot
        if (avatarId == 2) {
            virusTimer += dt;
            if (virusTimer >= 2.f) {
                virusTimer = 0.f;
                pending.sabotageIndices.clear();
                if (textLen > 0) {
                    for (int i = 0; i < 5; ++i)
                        pending.sabotageIndices.insert(std::rand() % textLen);
                }
            }
        } else {
            pending.sabotageIndices.clear();
        }

        // --- Commit the step's results to the shared shadow buffer ---
        // PARALLELISM [Phase 6 — Synchronization Point]:
        // This is the ONLY place in simLoop() where a mutex is acquired.
        // All other work above is purely local.  Keeping the critical section
        // this small is the key insight from Phase 3's shared-resource analysis.
        swapSnapshot();

        // Yield to avoid spinning at 100% CPU when simulation is fast
        std::this_thread::sleep_for(std::chrono::milliseconds(8)); // ~120 Hz sim rate
    }
    // simLoop exits → simThread becomes joinable → stop() can return
}

void NetworkSim::swapSnapshot() {
    // PARALLELISM [Phase 4 — Threading Grammar / Phase 5 — RAII Mutex]:
    // std::lock_guard acquires shadowMutex on construction and releases it
    // on destruction (end of this function scope).
    //
    // BNF rule:  <state-swap> ::= { std::lock_guard<std::mutex> lk(shadowMutex);
    //                               shadow = pending; }
    //
    // This is the C++ RAII pattern described in the framework's Phase 5
    // "C++ environment" section: zero-cost abstraction that auto-releases on
    // exception, making deadlocks from forgotten unlocks impossible.
    std::lock_guard<std::mutex> lk(shadowMutex);
    shadow = pending;
    pending.damageAccum = 0.f;  // reset accumulator after it has been committed
}

// =============================================================================
// tick() — called from the MAIN THREAD every frame
// =============================================================================
// PARALLELISM [Phase 2 — Sequential Baseline → Pipeline]:
// Previously tick() did ALL the simulation work here on the main thread.
// Now it only reads the latest committed snapshot (one brief mutex lock) and
// returns the accumulated damage.  The heavy work has moved to simLoop().
// =============================================================================
float NetworkSim::tick(float dt, int textLen, bool /*localPlayerFrozen*/) {
    if (!connected.load() || textLen <= 0) return 0.f;

    // PARALLELISM [Phase 6 — Main-thread Read Under Lock]:
    // Lock is held only long enough to copy the snapshot struct.
    // The local copy `snap` is then used freely without the lock.
    SimSnapshot snap;
    {
        std::lock_guard<std::mutex> lk(shadowMutex);
        snap = shadow;
    }

    // Publish snapshot values to the public fields that GameScreen reads
    progressPct     = snap.progressPct;
    wpm             = snap.wpm;
    multiplier      = snap.multiplier;
    sabotageIndices = snap.sabotageIndices;

    // Return the damage that accumulated since the last main-thread read
    return snap.damageAccum;
}