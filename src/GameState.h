#pragma once
#include <string>
#include <vector>
#include <set>
#include <SFML/Graphics.hpp>
#include "Stats.h"
#include "Avatar.h"
#include "NetworkSim.h"

enum class Screen {
    Title,
    AvatarSelect,
    VS,
    Game,
    Result,
    Profile,
    Settings,
    SPSelect
};

enum class GameMode {
    None,
    VS,          // local 2-player sim
    ScoreAttack, // SP: one text, highest score
    Endless      // SP: infinite waves, boss every 10
};

struct PlayerProfile {
    std::string name    = "UNIT ALPHA";
    int avatarId        = 0;
    int colorIndex      = 0;
    int gamesPlayed     = 0;
    int wins            = 0;
    int losses          = 0;
    int bestWpm         = 0;
    int bestScore       = 0;
    int avgAcc          = 100;
};

struct AppSettings {
    float musicVolume   = 0.6f;
    float sfxVolume     = 0.8f;
    bool scanlines      = true;
    bool darkMode       = true;
    bool glitchEffects  = true;
    float timeLimit     = 60.f;
};

struct GameState {
    Screen       currentScreen = Screen::Title;
    GameMode     mode          = GameMode::None;
    bool         selectingForMode = false;

    PlayerProfile profile;
    AppSettings   settings;

    // Per-match state
    int          round         = 1;
    int          maxRounds     = 3;
    int          wave          = 1;
    float        playerHP      = 100.f;
    float        oppHP         = 100.f;
    long long    score         = 0;
    float        multiplier    = 1.f;
    int          comboStreak   = 0;
    bool         roundStarted  = false;
    bool         matchFinished = false;
    bool         playerWon     = false;
    bool         isBossWave    = false;

    // FIX #7: accumulate stats across all rounds so result screen shows match totals
    int          matchTotalTyped   = 0;
    int          matchCorrectChars = 0;
    int          matchErrors       = 0;
    int          matchBestWpm      = 0;

    // Typing
    std::string  targetText;
    std::string  typedText;
    std::set<int> obfuscatedIndices;

    // FIX #8: frozenUntil is set at round START (in GameScreen::onEnter), not resetMatch
    std::chrono::time_point<std::chrono::steady_clock> frozenUntil;
    bool         opponentFrozen = false;

    Stats        stats;
    NetworkSim   opponent;

    sf::Font     fontMono;
    sf::Font     fontOrb;
    sf::Font     fontRaj;

    static constexpr sf::Color ACCENT_COLORS[] = {
        {0, 240, 255}, {255, 60, 110}, {240, 192, 0},
        {123, 47, 255}, {0, 230, 118}, {255, 145, 0}
    };

    void resetMatch();
    void applyDamageToPlayer(float dmg);
    void applyDamageToOpponent(float dmg);
    void updateMultiplier(bool correct);
    void addScore(long long pts);
    sf::Color playerAccentColor() const;
};
