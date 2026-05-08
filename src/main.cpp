#include <SFML/Graphics.hpp>
#include <memory>
#include <map>
#include <cstdlib>
#include <ctime>

// =============================================================================
// PARALLELISM [Phase 5 — Tool Selection]: Standard library concurrency headers
// =============================================================================
// All threading is done through the C++17 standard library — no external
// dependencies.  This aligns with the framework's Phase 5 recommendation for
// C++ environments: use RAII lock_guards and atomics for zero-cost, safe
// concurrency rather than raw POSIX pthread calls.
//
// Threads used in this program:
//   1. Main thread      — event handling, game logic, SFML rendering
//   2. sim thread       — NetworkSim::simLoop() — CPU opponent simulation
//   3-4. Cache threads  — refreshGlitchCache() worker pair (short-lived)
// =============================================================================

#include "GameState.h"
#include "Screens/IScreen.h"
#include "Screens/TitleScreen.h"
#include "Screens/AvatarSelectScreen.h"
#include "Screens/VSScreen.h"
#include "Screens/GameScreen.h"
#include "Screens/ResultScreen.h"
#include "Screens/ProfileScreen.h"
#include "Screens/SettingsScreen.h"
#include "Screens/SPSelectScreen.h"

static bool tryLoadFont(sf::Font& font, const std::vector<std::string>& paths) {
    for (const auto& p : paths)
        if (font.loadFromFile(p)) return true;
    return false;
}

int main() {
    std::srand((unsigned)std::time(nullptr));

    sf::RenderWindow window(sf::VideoMode(800, 600), "SYN//TYPE  -  ARCADE COMBAT TYPING",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    GameState gs;

    // Font loading (unchanged)
    if (!tryLoadFont(gs.fontMono, {
            "assets/ShareTechMono-Regular.ttf",
            "C:/Windows/Fonts/consola.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
            "/System/Library/Fonts/Menlo.ttc"
        })) {}

    if (!tryLoadFont(gs.fontOrb, {
            "assets/Orbitron-Bold.ttf",
            "assets/ShareTechMono-Regular.ttf",
            "C:/Windows/Fonts/consola.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
        })) {
        gs.fontOrb = gs.fontMono;
    }

    if (!tryLoadFont(gs.fontRaj, {
            "assets/Rajdhani-SemiBold.ttf",
            "assets/ShareTechMono-Regular.ttf",
            "C:/Windows/Fonts/consola.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
        })) {
        gs.fontRaj = gs.fontMono;
    }

    // Screen registry
    std::map<Screen, std::unique_ptr<IScreen>> screens;
    screens[Screen::Title]        = std::make_unique<TitleScreen>();
    screens[Screen::AvatarSelect] = std::make_unique<AvatarSelectScreen>();
    screens[Screen::VS]           = std::make_unique<VSScreen>();
    screens[Screen::Game]         = std::make_unique<GameScreen>();
    screens[Screen::Result]       = std::make_unique<ResultScreen>();
    screens[Screen::Profile]      = std::make_unique<ProfileScreen>();
    screens[Screen::Settings]     = std::make_unique<SettingsScreen>();
    screens[Screen::SPSelect]     = std::make_unique<SPSelectScreen>();

    Screen activeScreen = Screen::Title;
    screens[activeScreen]->onEnter(gs);

    sf::Clock clock;

    // =========================================================================
    // PARALLELISM [Phase 2 — Sequential Baseline → Parallel Main Loop]:
    //
    // BEFORE (original sequential flow per frame):
    //   [events] → [update + sim tick (blocking)] → [draw] → [display]
    //
    // AFTER (parallel flow):
    //   Main thread: [events] → [read snapshot] → [draw] → [display]
    //   Sim thread:  ↕ running concurrently ↕
    //
    // The main loop below is IDENTICAL to the original — the parallelism is
    // encapsulated inside NetworkSim and GameScreen.  The framework's Phase 6
    // principle: "integrate threading tools at the precise points identified
    // in Phase 3", not globally across the whole codebase.
    // =========================================================================
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;  // cap delta time on long frames

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                // PARALLELISM [Phase 6 — Clean Shutdown]:
                // Before closing the window, stop the sim thread if it is
                // running.  Destroying a joinable std::thread calls
                // std::terminate() — an unconditional process abort.
                // Calling stop() here ensures a graceful exit regardless of
                // which screen is active when the user clicks the X button.
                gs.opponent.stop();
                window.close();
                break;
            }
            if (screens.count(activeScreen))
                screens[activeScreen]->handleEvent(event, gs);
        }

        if (!window.isOpen()) break;

        if (gs.currentScreen != activeScreen) {
            // PARALLELISM [Phase 6 — Thread Lifecycle on Screen Transition]:
            // When leaving the Game screen, the sim thread must be stopped
            // before the new screen's onEnter() runs.  GameScreen::handleEvent
            // calls gs.opponent.stop() on Escape; for other transitions
            // (e.g. round completing and jumping to ResultScreen) endMatch()
            // already calls stop().  This is a belt-and-suspenders guard.
            if (activeScreen == Screen::Game)
                gs.opponent.stop();

            activeScreen = gs.currentScreen;
            if (screens.count(activeScreen))
                screens[activeScreen]->onEnter(gs);
        }

        if (screens.count(activeScreen)) {
            screens[activeScreen]->update(dt, gs);
            screens[activeScreen]->draw(window, gs);
        }

        window.display();
    }

    // PARALLELISM [Phase 7 — Evaluation]:
    // Final join guard: ensures simThread (if somehow still running) is
    // cleanly stopped before main() returns and stack objects are destroyed.
    // Without this, the OS may reclaim memory while the thread is still
    // accessing gs — undefined behavior that ThreadSanitizer will flag.
    gs.opponent.stop();

    return 0;
}