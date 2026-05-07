#include "GameState.h"
#include "TextSnippets.h"
#include <algorithm>
#include <cmath>
#include <chrono>

void GameState::resetMatch() {
    round          = 1;
    playerHP       = 100.f;
    oppHP          = 100.f;
    score          = 0;
    multiplier     = 1.f;
    comboStreak    = 0;
    roundStarted   = false;
    matchFinished  = false;
    playerWon      = false;
    isBossWave     = mode == GameMode::Endless && (wave % 10 == 0);
    obfuscatedIndices.clear();
    opponentFrozen = false;
    stats.timeLimit = settings.timeLimit;
    stats.reset();

    // FIX #7: reset match-total accumulators
    matchTotalTyped   = 0;
    matchCorrectChars = 0;
    matchErrors       = 0;
    matchBestWpm      = 0;

    auto snippets = isBossWave ? TextSnippets::boss() : TextSnippets::normal();
    targetText = TextSnippets::getRandom(snippets);
    typedText  = "";

    int oppAv = (profile.avatarId + 2) % 4;

    // FIX #8: DO NOT apply Sentinel freeze here — apply it in GameScreen::onEnter
    // so the 4-second freeze starts when the game screen appears, not during the
    // VS countdown animation.
    frozenUntil    = std::chrono::steady_clock::now(); // safe default (already expired)

    if (mode == GameMode::VS) {
        opponent.reset(oppAv, false);
    }
}

void GameState::applyDamageToPlayer(float dmg) {
    playerHP = std::max(0.f, playerHP - dmg);
}

void GameState::applyDamageToOpponent(float dmg) {
    oppHP = std::max(0.f, oppHP - dmg);
}

void GameState::updateMultiplier(bool correct) {
    auto avatars = Avatar::all();
    const Avatar& av = avatars[profile.avatarId];

    if (correct) {
        comboStreak++;
        if (comboStreak >= 10) {
            multiplier = std::min(multiplier + av.multiGainRate, av.maxMultiplier);
        }
        // Phoenix round 3 bonus
        if (av.ability == AvatarAbility::Comeback && round == 3) {
            multiplier = std::min(multiplier + av.multiGainRate * 0.5f, av.maxMultiplier);
        }
    } else {
        comboStreak = 0;
        float acc = stats.totalTyped > 0
            ? (float)stats.correctChars / stats.totalTyped * 100.f : 100.f;
        if (acc < av.accuracyThreshold) {
            if (av.ability == AvatarAbility::Volatile) {
                multiplier = 1.f;
            } else {
                multiplier = std::max(1.f, multiplier - 0.5f);
            }
        }
    }
}

void GameState::addScore(long long pts) {
    score += (long long)(pts * multiplier);
}

sf::Color GameState::playerAccentColor() const {
    return ACCENT_COLORS[profile.colorIndex % 6];
}
