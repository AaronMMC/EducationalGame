#include "TitleScreen.h"
#include "../UIHelpers.h"
#include <cmath>

void TitleScreen::onEnter(GameState& gs) {
    animTime   = 0.f;
    hoveredBtn = -1;
}

void TitleScreen::handleEvent(sf::Event& event, GameState& gs) {
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i m(event.mouseButton.x, event.mouseButton.y);

        // VS (Multiplayer)
        if (m.x >= 250 && m.x <= 550 && m.y >= 240 && m.y <= 285) {
            gs.mode = GameMode::VS;
            gs.wave = 1;
            gs.currentScreen = Screen::AvatarSelect;
        }
        // Single Player
        if (m.x >= 250 && m.x <= 550 && m.y >= 300 && m.y <= 345) {
            gs.selectingForMode = true;
            gs.currentScreen = Screen::SPSelect;
        }
        // Profile
        if (m.x >= 250 && m.x <= 550 && m.y >= 360 && m.y <= 405) {
            gs.currentScreen = Screen::Profile;
        }
        // Settings
        if (m.x >= 250 && m.x <= 550 && m.y >= 420 && m.y <= 465) {
            gs.currentScreen = Screen::Settings;
        }
    }
}

void TitleScreen::update(float dt, GameState& gs) {
    animTime += dt;
}

void TitleScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);

    // Animated grid background
    for (int gx = 0; gx < 800; gx += 40) {
        sf::RectangleShape line({1.f, 600.f});
        line.setPosition((float)gx, 0.f);
        float alpha = 12.f + 6.f * std::sin(animTime * 0.5f + gx * 0.01f);
        line.setFillColor({0, 240, 255, (sf::Uint8)alpha});
        w.draw(line);
    }
    for (int gy = 0; gy < 600; gy += 40) {
        sf::RectangleShape line({800.f, 1.f});
        line.setPosition(0.f, (float)gy);
        float alpha = 12.f + 6.f * std::sin(animTime * 0.5f + gy * 0.01f);
        line.setFillColor({0, 240, 255, (sf::Uint8)alpha});
        w.draw(line);
    }

    // Logo
    sf::Text logo = UI::makeText("SYN//TYPE", gs.fontOrb, 52, UI::ACCENT);
    logo.setStyle(sf::Text::Bold);
    UI::centerText(logo, 400.f, 100.f);
    UI::drawGlowText(w, logo, UI::ACCENT, 8.f);

    sf::Text sub = UI::makeText("ARCADE COMBAT TYPING", gs.fontMono, 13, UI::TEXT2);
    UI::centerText(sub, 400.f, 148.f);
    w.draw(sub);

    // Animated underline
    float lineW = 200.f + 40.f * std::sin(animTime * 2.f);
    sf::RectangleShape underline({lineW, 2.f});
    underline.setPosition(400.f - lineW / 2.f, 162.f);
    underline.setFillColor(UI::ACCENT);
    w.draw(underline);

    sf::Vector2i mouse = sf::Mouse::getPosition(w);

    // Button definitions
    struct Btn { std::string label; float y; sf::Color col; };
    std::vector<Btn> btns = {
        {"[VS]  MULTIPLAYER",  252.f, UI::ACCENT2},
        {"[SP]  SINGLE PLAYER",312.f, UI::ACCENT},
        {"[PR]  PROFILE",      372.f, UI::ACCENT3},
        {"[ST]  SETTINGS",     432.f, UI::TEXT2},
    };

    for (int i = 0; i < (int)btns.size(); ++i) {
        bool hov = (mouse.x >= 250 && mouse.x <= 550 &&
                    mouse.y >= btns[i].y - 12 && mouse.y <= btns[i].y + 33);

        sf::RectangleShape bg({300.f, 45.f});
        bg.setPosition(250.f, btns[i].y - 12.f);
        sf::Color bCol = btns[i].col;
        bg.setFillColor({(sf::Uint8)(bCol.r/10), (sf::Uint8)(bCol.g/10), (sf::Uint8)(bCol.b/10)});
        bg.setOutlineColor(hov ? bCol : sf::Color(bCol.r, bCol.g, bCol.b, 80));
        bg.setOutlineThickness(1.f);
        w.draw(bg);

        sf::Text btn = UI::makeText(btns[i].label, gs.fontOrb, 15, hov ? bCol : UI::TEXT2);
        UI::centerText(btn, 400.f, btns[i].y + 10.f);
        if (hov) UI::drawGlowText(w, btn, bCol, 3.f);
        else w.draw(btn);
    }

    // Avatar preview strip
    auto avatars = Avatar::all();
    float ax = 80.f;
    for (auto& av : avatars) {
        sf::Text sym = UI::makeText(av.symbol, gs.fontMono, 11, av.accentColor);
        sym.setPosition(ax, 520.f);
        w.draw(sym);

        float pulse = 0.7f + 0.3f * std::sin(animTime * 1.5f + ax * 0.05f);
        sf::RectangleShape dot({60.f, 2.f});
        dot.setPosition(ax, 534.f);
        dot.setFillColor({av.accentColor.r, av.accentColor.g, av.accentColor.b,
                          (sf::Uint8)(pulse * 180)});
        w.draw(dot);
        ax += 160.f;
    }

    sf::Text ver = UI::makeText("v1.0  //  MIT", gs.fontMono, 9, UI::TEXT3);
    ver.setPosition(10.f, 582.f);
    w.draw(ver);

    UI::drawCornerBrackets(w, 8, 8, 792, 592, UI::ACCENT, 18.f);
    if (gs.settings.scanlines) UI::drawScanlines(w);
}
