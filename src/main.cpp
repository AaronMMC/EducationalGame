#include <SFML/Graphics.hpp>
#include <memory>
#include <iostream>
#include <cstdlib>
#include <ctime>

#include "GameState.h"
#include "Screens/TitleScreen.h"
#include "Screens/AvatarSelectScreen.h"
#include "Screens/VSScreen.h"
#include "Screens/GameScreen.h"
#include "Screens/ResultScreen.h"
#include "Screens/ProfileScreen.h"
#include "Screens/SettingsScreen.h"
#include "Screens/SPSelectScreen.h"

// ── Font loading helper ────────────────────────────────────────────────────────
static bool loadFont(sf::Font& font, const std::vector<std::string>& paths) {
    for (const auto& p : paths)
        if (font.loadFromFile(p)) return true;
    return false;
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // ── Window ────────────────────────────────────────────────────────────────
    sf::RenderWindow window(sf::VideoMode(800, 600), "SynType Arcade",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    // ── State ─────────────────────────────────────────────────────────────────
    GameState gs;

    // ── Fonts ─────────────────────────────────────────────────────────────────
    // Monospace / code font
    if (!loadFont(gs.fontMono, {
        "assets/ShareTechMono-Regular.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/cour.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/System/Library/Fonts/Menlo.ttc"
    })) {
        std::cerr << "[SynType] WARNING: Could not load mono font. "
                     "Place ShareTechMono-Regular.ttf in assets/\n";
    }

    // Display / heading font — fallback to mono if not found
    if (!loadFont(gs.fontOrb, {
        "assets/Orbitron-Bold.ttf",
        "assets/Orbitron-Regular.ttf",
        "C:/Windows/Fonts/impact.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",
        "/System/Library/Fonts/Supplemental/Impact.ttf"
    })) {
        gs.fontOrb = gs.fontMono; // graceful fallback
    }

    // Body font
    if (!loadFont(gs.fontRaj, {
        "assets/Rajdhani-SemiBold.ttf",
        "assets/Rajdhani-Regular.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/System/Library/Fonts/Helvetica.ttc"
    })) {
        gs.fontRaj = gs.fontMono;
    }

    // ── Screens ───────────────────────────────────────────────────────────────
    auto titleScr    = std::make_unique<TitleScreen>();
    auto avatarScr   = std::make_unique<AvatarSelectScreen>();
    auto vsScr       = std::make_unique<VSScreen>();
    auto gameScr     = std::make_unique<GameScreen>();
    auto resultScr   = std::make_unique<ResultScreen>();
    auto profileScr  = std::make_unique<ProfileScreen>();
    auto settingsScr = std::make_unique<SettingsScreen>();
    auto spScr       = std::make_unique<SPSelectScreen>();

    Screen prevScreen = Screen::Title;
    titleScr->onEnter(gs);

    // ── Main loop ─────────────────────────────────────────────────────────────
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.05f); // clamp in case of frame spike

        // ── Screen change detection ───────────────────────────────────────────
        if (gs.currentScreen != prevScreen) {
            switch (gs.currentScreen) {
                case Screen::Title:       titleScr->onEnter(gs);   break;
                case Screen::AvatarSelect:avatarScr->onEnter(gs);  break;
                case Screen::VS:          vsScr->onEnter(gs);      break;
                case Screen::Game:        gameScr->onEnter(gs);    break;
                case Screen::Result:      resultScr->onEnter(gs);  break;
                case Screen::Profile:     profileScr->onEnter(gs); break;
                case Screen::Settings:    /* no onEnter needed */  break;
                case Screen::SPSelect:    /* no onEnter needed */  break;
            }
            prevScreen = gs.currentScreen;
        }

        // ── Events ────────────────────────────────────────────────────────────
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            switch (gs.currentScreen) {
                case Screen::Title:        titleScr->handleEvent(event, gs);    break;
                case Screen::AvatarSelect: avatarScr->handleEvent(event, gs);   break;
                case Screen::VS:           vsScr->handleEvent(event, gs);       break;
                case Screen::Game:         gameScr->handleEvent(event, gs);     break;
                case Screen::Result:       resultScr->handleEvent(event, gs);   break;
                case Screen::Profile:      profileScr->handleEvent(event, gs);  break;
                case Screen::Settings:     settingsScr->handleEvent(event, gs); break;
                case Screen::SPSelect:     spScr->handleEvent(event, gs);       break;
            }
        }

        // ── Update ────────────────────────────────────────────────────────────
        switch (gs.currentScreen) {
            case Screen::Title:        titleScr->update(dt, gs);    break;
            case Screen::AvatarSelect: avatarScr->update(dt, gs);   break;
            case Screen::VS:           vsScr->update(dt, gs);       break;
            case Screen::Game:         gameScr->update(dt, gs);     break;
            case Screen::Result:       resultScr->update(dt, gs);   break;
            case Screen::Profile:      profileScr->update(dt, gs);  break;
            case Screen::Settings:     settingsScr->update(dt, gs); break;
            case Screen::SPSelect:     spScr->update(dt, gs);       break;
        }

        // ── Draw ──────────────────────────────────────────────────────────────
        switch (gs.currentScreen) {
            case Screen::Title:        titleScr->draw(window, gs);    break;
            case Screen::AvatarSelect: avatarScr->draw(window, gs);   break;
            case Screen::VS:           vsScr->draw(window, gs);       break;
            case Screen::Game:         gameScr->draw(window, gs);     break;
            case Screen::Result:       resultScr->draw(window, gs);   break;
            case Screen::Profile:      profileScr->draw(window, gs);  break;
            case Screen::Settings:     settingsScr->draw(window, gs); break;
            case Screen::SPSelect:     spScr->draw(window, gs);       break;
        }

        window.display();
    }

    return 0;
}
