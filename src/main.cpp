#include <SFML/Graphics.hpp>
#include <memory>
#include <map>
#include <cstdlib>
#include <ctime>

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

// Try to load a font from several candidate paths.
// Falls back gracefully if none are found.
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

    // ── Font loading ──────────────────────────────────────────────────────────
    // Mono font (Share Tech Mono preferred, fallback to system)
    if (!tryLoadFont(gs.fontMono, {
            "assets/ShareTechMono-Regular.ttf",
            "C:/Windows/Fonts/consola.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
            "/System/Library/Fonts/Menlo.ttc"
        })) {
        // SFML will use the default built-in font if nothing loads;
        // the game still runs, just without the themed font.
    }

    // Display / title font (Orbitron preferred)
    if (!tryLoadFont(gs.fontOrb, {
            "assets/Orbitron-Bold.ttf",
            "assets/ShareTechMono-Regular.ttf",   // acceptable fallback
            "C:/Windows/Fonts/consola.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
        })) {
        gs.fontOrb = gs.fontMono; // reuse mono if nothing else works
    }

    // Subtitle / tagline font (Rajdhani preferred)
    if (!tryLoadFont(gs.fontRaj, {
            "assets/Rajdhani-SemiBold.ttf",
            "assets/ShareTechMono-Regular.ttf",
            "C:/Windows/Fonts/consola.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
        })) {
        gs.fontRaj = gs.fontMono;
    }

    // ── Screen registry ───────────────────────────────────────────────────────
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

    // ── Main loop ─────────────────────────────────────────────────────────────
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        // Cap dt so a long frame (e.g. first-time font load) doesn't teleport the sim
        if (dt > 0.1f) dt = 0.1f;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                break;
            }
            if (screens.count(activeScreen))
                screens[activeScreen]->handleEvent(event, gs);
        }

        if (!window.isOpen()) break;

        // Detect screen transition
        if (gs.currentScreen != activeScreen) {
            activeScreen = gs.currentScreen;
            if (screens.count(activeScreen))
                screens[activeScreen]->onEnter(gs);
        }

        // Update & draw
        if (screens.count(activeScreen)) {
            screens[activeScreen]->update(dt, gs);
            screens[activeScreen]->draw(window, gs);
        }

        window.display();
    }

    return 0;
}
