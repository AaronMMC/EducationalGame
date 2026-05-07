#include "NetworkSim.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

void NetworkSim::reset(int avId, bool frozenStart) {
    avatarId      = avId;
    progressPct   = 0.f;
    wpm           = 0;
    accuracy      = 95;
    hp            = 100.f;
    multiplier    = 1.f;
    frozen        = frozenStart;
    charsTyped    = 0.f;
    virusTimer    = 0.f;
    glitchTimer   = 0.f;
    started       = false;
    lastMilestone = 0;          // FIX #4: reset milestone tracker
    sabotageIndices.clear();
    // Random speed between ~35 and ~65 WPM equivalent (chars/sec)
    oppSpeed  = (35.f + (std::rand() % 30)) / 60.f * 5.f;
    connected = true;
}

float NetworkSim::tick(float dt, int textLen, bool localPlayerFrozen) {
    if (!connected || textLen <= 0) return 0.f;

    if (!started) {
        startTime = std::chrono::steady_clock::now();
        started   = true;
    }

    float damage = 0.f;

    if (!frozen) {
        float jitter  = 1.f + (std::rand() % 30 - 15) / 100.f;
        charsTyped   += oppSpeed * dt * jitter;
        charsTyped    = std::min(charsTyped, (float)textLen);
        progressPct   = charsTyped / textLen;

        float elapsed = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - startTime).count() / 60.f;
        wpm = elapsed > 0.f ? (int)((charsTyped / 5.f) / elapsed) : 0;

        multiplier = std::min(multiplier + 0.01f * dt, 4.f);

        // FIX #4: use integer milestone tracking instead of modulo on float
        int currentMilestone = (int)charsTyped / 15;
        if (currentMilestone > lastMilestone && charsTyped > 1.f) {
            lastMilestone = currentMilestone;
            damage = 1.f;
        }
    }

    // Virus ability: inject sabotage indices onto local screen
    if (avatarId == 2) {
        virusTimer += dt;
        if (virusTimer >= 2.f) {
            virusTimer = 0.f;
            sabotageIndices.clear();
            for (int i = 0; i < 5; ++i)
                sabotageIndices.insert(std::rand() % textLen);
        }
    } else {
        sabotageIndices.clear();
    }

    return damage;
}
