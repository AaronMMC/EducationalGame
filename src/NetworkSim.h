#pragma once
#include <string>
#include <vector>
#include <set>
#include <thread>           // PARALLELISM [Phase 5]: std::thread — background sim loop
#include <mutex>            // PARALLELISM [Phase 5]: std::mutex — protects shadow state buffer
#include <atomic>           // PARALLELISM [Phase 5]: std::atomic — lock-free stop flag
#include <chrono>
#include "Avatar.h"
#include "Stats.h"

// =============================================================================
// PARALLELISM DESIGN — NetworkSim (Pipeline Pattern)
// =============================================================================
//
// PHASE 1 — Bottleneck Identification:
//   NetworkSim::tick() is called every frame on the main thread.  In the
//   original sequential design this blocks the render pipeline for as long as
//   the simulation logic takes.  As the CPU opponent logic grows (pathfinding,
//   AI tuning, real network I/O), this becomes a measurable stall.
//
// PHASE 2 — Sequential Baseline Data Dependencies:
//   tick() writes:  progressPct, wpm, accuracy, multiplier, sabotageIndices
//   draw()  reads:  all of the above (every frame)
//   → Classic Producer → Consumer dependency.
//
// PHASE 3 — Pattern Selection:
//   "Can these tasks run independently?" → YES, after the frame's state is
//   committed.  We use the PIPELINE pattern:
//
//     Sim thread  ──produces──▶  shadow buffer  ──consumed by──▶  main thread
//
//   The shadow buffer is a plain struct (SimSnapshot) copied under a mutex.
//   The main thread reads the snapshot without holding the lock long, so
//   rendering is never blocked longer than one memcpy.
//
// PHASE 4 — Threading Grammar (BNF excerpt):
//   <sim-thread>  ::= std::thread( &NetworkSim::simLoop, this )
//   <stop-flag>   ::= std::atomic<bool> stopFlag { false }
//   <state-swap>  ::= { std::lock_guard<std::mutex> lk(shadowMutex);
//                       shadow = pending; }
//
// PHASE 6 — Integration Points:
//   • connect()  → launches simThread
//   • reset()    → sets stopFlag, joins simThread, relaunches
//   • ~NetworkSim() / explicit stop() → joins cleanly
// =============================================================================

// -----------------------------------------------------------------------------
// SimSnapshot — the data produced by the sim thread and consumed by the
//               render/game thread.  Copying this struct under a mutex is the
//               entire synchronization cost per frame.
// -----------------------------------------------------------------------------
struct SimSnapshot {
    float progressPct  = 0.f;
    int   wpm          = 0;
    int   accuracy     = 95;
    float multiplier   = 1.f;
    float damageAccum  = 0.f;       // damage queued since last main-thread read
    std::set<int> sabotageIndices;  // VIRUS: chars to obfuscate this tick
};

class NetworkSim {
public:
    // -------------------------------------------------------------------------
    // Public API — called from the MAIN thread only
    // -------------------------------------------------------------------------

    int   avatarId    = 1;

    // PARALLELISM [Phase 3 — Shared Resource]:
    // These are the last-committed snapshot values.  The main thread reads
    // them; the sim thread writes them only via swapSnapshot().  Always read
    // them inside a lock_guard(shadowMutex) scope.
    float progressPct = 0.f;
    int   wpm         = 0;
    int   accuracy    = 95;
    float hp          = 100.f;
    float multiplier  = 1.f;

    // PARALLELISM [Phase 5 — Atomic flag]:
    // frozen is set by the main thread (Sentinel ability) and read by the sim
    // thread.  std::atomic<bool> ensures the write is visible across cores
    // without a mutex acquire.
    std::atomic<bool> frozen { false };

    std::set<int> sabotageIndices;

    void  reset(int avId, bool frozenStart);

    // tick() — main-thread entry point.
    // Instead of doing the simulation inline, this now just:
    //   1. Locks shadowMutex briefly
    //   2. Copies the latest SimSnapshot from the sim thread
    //   3. Returns accumulated damage
    // The actual simulation runs concurrently in simThread.
    float tick(float dt, int textLen, bool localPlayerFrozen);

    // stop() — signals simThread to exit and joins it.
    // Must be called before destroying NetworkSim (or on match end).
    void stop();

    bool isConnected() const { return connected.load(); }

    // connect() — launches the background sim thread.
    // PARALLELISM [Phase 6 — Integration]: called from GameScreen::startRound()
    // at the moment the player types the first character, so the thread
    // lifecycle is tightly scoped to an active match.
    void connect();

    ~NetworkSim() { stop(); }

private:
    // -------------------------------------------------------------------------
    // Sim-thread state — ONLY touched inside simLoop()
    // -------------------------------------------------------------------------
    float charsTyped    = 0.f;
    float virusTimer    = 0.f;
    float oppSpeed      = 0.f;
    int   lastMilestone = 0;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    bool  started       = false;

    // -------------------------------------------------------------------------
    // PARALLELISM [Phase 4 — Threading Grammar]:
    // Shadow buffer — the sim thread writes to `pending`; the main thread
    // reads from `shadow`.  The swap happens under shadowMutex using RAII
    // lock_guard so no unlock can be forgotten.
    // -------------------------------------------------------------------------
    SimSnapshot pending;   // sim-thread-owned working copy
    SimSnapshot shadow;    // last committed snapshot — main thread reads this

    // PARALLELISM [Phase 5 — Mutex]:
    // shadowMutex guards the copy of pending → shadow.
    // It is only held for the duration of a struct copy (~microseconds),
    // so the main thread is never blocked meaningfully.
    std::mutex shadowMutex;

    // PARALLELISM [Phase 5 — Atomic stop flag]:
    // stopFlag is set to true by the main thread (via stop()/reset()) and
    // polled by simLoop().  Using atomic avoids a mutex just for a bool check
    // at the top of the sim loop — the exact lock-free pattern from Phase 5.
    std::atomic<bool> stopFlag  { false };
    std::atomic<bool> connected { false };

    // PARALLELISM [Phase 4 — Thread object]:
    // simThread runs simLoop() concurrently with the main render loop.
    std::thread simThread;

    // The background simulation loop — runs on simThread.
    // Mirrors the original tick() logic but operates on local `pending`
    // state, then swaps it into `shadow` under the lock.
    void simLoop(int textLen);

    // swapSnapshot() — called inside simLoop() after each simulated step.
    // PARALLELISM [Phase 6 — Synchronization Point]:
    // This is the precise integration point identified in Phase 3 where
    // two threads touch shared data.  The lock_guard scope is intentionally
    // kept as small as possible (one struct copy) to minimise contention.
    void swapSnapshot();
};