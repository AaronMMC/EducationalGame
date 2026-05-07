#include "NetworkSim.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

void NetworkSim::reset(int avId, bool frozenStart) {
    avatarId     = avId;
    progressPct  = 0.f;
    wpm          = 0;
    accuracy     = 95;
    hp           = 100.f;
    multiplier   = 1.f;
    frozen       = frozenStart;
    charsTyped   = 0.f;
    virusTimer   = 0.f;
    started      = false;
    sabotageIndices.clear();
    // Random speed between ~35 and ~65 WPM equivalent (chars/sec)
    oppSpeed = (35.f + (std::rand() % 30)) / 60.f * 5.f;
    connected = true;
}

float NetworkSim::tick(float dt, int textLen, bool localPlayerFrozen) {
    if (!connected || textLen <= 0) return 0.f;

    // CPU opponent starts after a short delay
    if (!started) {
        startTime = std::chrono::steady_clock::now();
        started   = true;
    }

    float damage = 0.f;

    if (!frozen) {
        // Advance typing
        float jitter  = 1.f + (std::rand() % 30 - 15) / 100.f;
        charsTyped   += oppSpeed * dt * jitter;
        charsTyped    = std::min(charsTyped, (float)textLen);
        progressPct   = charsTyped / textLen;

        // Recalc WPM
        float elapsed = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - startTime).count() / 60.f;
        wpm = elapsed > 0.f ? (int)((charsTyped / 5.f) / elapsed) : 0;

        // Multiplier simulation — grows slowly
        multiplier = std::min(multiplier + 0.01f * dt, 4.f);

        // Deal passive chip damage every ~3s worth of typing
        if ((int)charsTyped % 15 == 0 && charsTyped > 1.f) {
            damage = 1.f;
        }
    }

    // Virus ability: inject sabotage indices onto local screen (avatarId==2 is VIRUS)
    if (avatarId == 2) {
        virusTimer += dt;
        if (virusTimer >= 2.f) {
            virusTimer = 0.f;
            sabotageIndices.clear();
            for (int i = 0; i < 5; ++i) {
                int idx = std::rand() % textLen;
                sabotageIndices.insert(idx);
            }
        }
    } else {
        sabotageIndices.clear();
    }

    return damage;
}

