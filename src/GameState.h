#pragma once
#include <string>
#include <vector>
#include <set>
#include <mutex>        // PARALLELISM [Phase 5]: std::mutex for RAII lock_guard protection
#include <atomic>       // PARALLELISM [Phase 5]: std::atomic for lock-free flags (sim ↔ main thread)
#include <SFML/Graphics.hpp>
#include "Stats.h"
#include "Avatar.h"
#include "NetworkSim.h"

// =============================================================================
// PARALLELISM NOTE — Phase 3 (Concept Mapping): Shared Resource Identification
// =============================================================================
// The following GameState members are accessed from TWO threads simultaneously:
//   • main thread  : reads them in update() and draw()
//   • sim thread   : writes them inside NetworkSim's background tick loop
//
// Per the Team 3 framework's 3rd mental-model question —
//   "Are two tasks changing the same variable?"
// — these fields require synchronization via a mutex (see simMutex below).
//
// Fields that are shared across threads:
//   - oppHP              (written by sim, read by drawHUD)
//   - playerHP           (written by sim on damage events, read by drawHUD)
//   - obfuscatedIndices  (written by sim every 2s, read by drawTypingArea)
//
// Fields that are thread-LOCAL (no sync needed):
//   - targetText         (set once per round before sim starts, then read-only)
//   - typedText          (only ever touched on the main thread via keyboard events)
//   - stats              (owned exclusively by the main thread)
// =============================================================================

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
    VS,
    ScoreAttack,
    Endless
};

struct PlayerProfile {
    std::string name       = "UNIT ALPHA";
    int         avatarId   = 0;
    int         colorIndex = 0;
    int         gamesPlayed = 0;
    int         wins        = 0;
    int         losses      = 0;
    int         bestWpm     = 0;
    int         bestScore   = 0;
    int         avgAcc      = 100;
};

struct AppSettings {
    float musicVolume  = 0.6f;
    float sfxVolume    = 0.8f;
    bool  scanlines    = true;
    bool  darkMode     = true;
    bool  glitchEffects = true;
    float timeLimit    = 60.f;
};

struct GameState {
    Screen    currentScreen   = Screen::Title;
    GameMode  mode            = GameMode::None;
    bool      selectingForMode = false;

    PlayerProfile profile;
    AppSettings   settings;

    // -------------------------------------------------------------------------
    // Per-match state
    // -------------------------------------------------------------------------
    int   round         = 1;
    int   maxRounds     = 3;
    int   wave          = 1;

    // PARALLELISM [Phase 3 — Shared Resources]:
    // playerHP and oppHP are written by NetworkSim's background thread (on
    // damage events) and read every frame by drawHUD on the main thread.
    // Always lock simMutex before reading or writing these two values.
    float playerHP      = 100.f;
    float oppHP         = 100.f;

    long long score     = 0;
    float multiplier    = 1.f;
    int   comboStreak   = 0;
    bool  roundStarted  = false;
    bool  matchFinished = false;
    bool  playerWon     = false;
    bool  isBossWave    = false;

    // Accumulated match stats across rounds (main-thread only, no mutex needed)
    int matchTotalTyped   = 0;
    int matchCorrectChars = 0;
    int matchErrors       = 0;
    int matchBestWpm      = 0;

    // -------------------------------------------------------------------------
    // Typing — main-thread only; no mutex required
    // -------------------------------------------------------------------------
    std::string   targetText;   // set once per round, read-only during play
    std::string   typedText;    // keyboard events only → main thread

    // PARALLELISM [Phase 3 — Shared Resources]:
    // obfuscatedIndices is populated by the VIRUS avatar's sabotage logic
    // inside NetworkSim::tick(), which runs on its own thread.  drawTypingArea
    // reads it every frame.  Must be accessed under simMutex.
    std::set<int> obfuscatedIndices;

    std::chrono::time_point<std::chrono::steady_clock> frozenUntil;

    // PARALLELISM [Phase 5 — Tool Selection]:
    // std::atomic<bool> chosen over a mutex-guarded bool because:
    //   • It is a single flag (no compound operation needed)
    //   • atomic loads/stores are lock-free on every major platform
    //   • avoids a mutex acquire/release for a one-bit check done every frame
    std::atomic<bool> opponentFrozen { false };

    // PARALLELISM [Phase 4 — Threading Grammar / Phase 5 — Tool Selection]:
    // simMutex protects all GameState fields that are written by NetworkSim's
    // background thread and read by the main (render) thread.
    //
    // Usage pattern — RAII lock_guard (auto-releases on scope exit, even on
    // exception; this is the zero-cost abstraction described in Phase 5):
    //
    //   { std::lock_guard<std::mutex> lock(gs.simMutex);
    //     float hp = gs.oppHP;   // safe read
    //   }
    //
    // Using lock_guard instead of raw mutex.lock()/unlock() prevents
    // deadlocks caused by forgotten unlocks — the exact failure mode the
    // framework warns about for C-style POSIX mutex usage.
    mutable std::mutex simMutex;

    Stats      stats;
    NetworkSim opponent;

    sf::Font fontMono;
    sf::Font fontOrb;
    sf::Font fontRaj;

    static const sf::Color ACCENT_COLORS[];

    void resetMatch();
    void applyDamageToPlayer(float dmg);    // must be called under simMutex
    void applyDamageToOpponent(float dmg);  // must be called under simMutex
    void updateMultiplier(bool correct);
    void addScore(long long pts);
    sf::Color playerAccentColor() const;
};