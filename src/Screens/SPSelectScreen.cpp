#include "SPSelectScreen.h"
#include "../UIHelpers.h"

static sf::FloatRect cardBounds(int i) {
    return {120.f + i * 280.f, 200.f, 240.f, 200.f};
}

void SPSelectScreen::handleEvent(sf::Event& event, GameState& gs) {
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2i m(event.mouseMove.x, event.mouseMove.y);
        hovered = -1;
        for (int i = 0; i < 2; ++i) {
            auto b = cardBounds(i);
            if (m.x >= b.left && m.x <= b.left+b.width && m.y >= b.top && m.y <= b.top+b.height)
                hovered = i;
        }
    }
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i m(event.mouseButton.x, event.mouseButton.y);
        for (int i = 0; i < 2; ++i) {
            auto b = cardBounds(i);
            if (m.x >= b.left && m.x <= b.left+b.width && m.y >= b.top && m.y <= b.top+b.height) {
                if (i == 0) {
                    gs.mode = GameMode::ScoreAttack;
                    gs.wave = 1;
                    gs.currentScreen = Screen::AvatarSelect;
                } else {
                    gs.mode = GameMode::Endless;
                    gs.wave = 1;
                    gs.currentScreen = Screen::AvatarSelect;
                }
            }
        }
        if (m.x >= 300 && m.x <= 500 && m.y >= 450 && m.y <= 490)
            gs.currentScreen = Screen::Title;
    }
    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::Escape)
        gs.currentScreen = Screen::Title;
}

void SPSelectScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);
    UI::drawCornerBrackets(w, 10, 10, 780, 580, UI::ACCENT, 20.f);

    sf::Text title = UI::makeText("SINGLE PLAYER", gs.fontOrb, 22, UI::ACCENT);
    UI::centerText(title, 400.f, 80.f);
    UI::drawGlowText(w, title, UI::ACCENT, 4.f);
    sf::Text sub = UI::makeText("SELECT COMBAT MODE", gs.fontMono, 11, UI::TEXT2);
    UI::centerText(sub, 400.f, 112.f);
    w.draw(sub);

    struct ModeInfo { std::string icon, name, desc; };
    std::vector<ModeInfo> modes = {
        {"[->]", "SCORE ATTACK",
         "Type one snippet as fast and accurately as possible. Multiplier active. One shot at a high score."},
        {"[oo]", "ENDLESS SIEGE",
         "Infinite waves. Every 10 waves triggers a Boss Battle — all four avatar AIs vs you. Gets harder each wave."},
    };

    for (int i = 0; i < 2; ++i) {
        bool hov = (hovered == i);
        auto b = cardBounds(i);
        UI::drawPanel(w, b.left, b.top, b.width, b.height,
                      hov ? UI::ACCENT : UI::TEXT3,
                      hov ? sf::Color(0,240,255,10) : UI::BG2, 10.f);

        sf::Text icon = UI::makeText(modes[i].icon, gs.fontMono, 24, hov ? UI::ACCENT : UI::TEXT2);
        UI::centerText(icon, b.left + b.width/2.f, b.top + 40.f);
        if (hov) UI::drawGlowText(w, icon, UI::ACCENT, 3.f);
        else w.draw(icon);

        sf::Text nm = UI::makeText(modes[i].name, gs.fontOrb, 14, hov ? UI::ACCENT : UI::TEXT);
        nm.setStyle(sf::Text::Bold);
        UI::centerText(nm, b.left + b.width/2.f, b.top + 80.f);
        w.draw(nm);

        // Wrap description
        std::string desc = modes[i].desc;
        float ty = b.top + 106.f;
        std::string word, line;
        auto flush = [&]() {
            sf::Text dt = UI::makeText(line, gs.fontMono, 9, UI::TEXT2);
            UI::centerText(dt, b.left + b.width/2.f, ty);
            w.draw(dt);
            ty += 14.f; line = "";
        };
        for (char c : desc + " ") {
            if (c == ' ') {
                if (line.size() + word.size() > 26) flush();
                if (!line.empty()) line += ' ';
                line += word; word = "";
            } else word += c;
        }
        if (!line.empty()) flush();
    }

    sf::Vector2i m = sf::Mouse::getPosition(w);
    bool hb = (m.x >= 300 && m.x <= 500 && m.y >= 450 && m.y <= 490);
    UI::drawPanel(w, 300, 450, 200, 40, hb ? UI::ACCENT2 : UI::TEXT2,
                  hb ? sf::Color(255,60,110,20) : UI::BG2, 8.f);
    sf::Text back = UI::makeText("BACK", gs.fontOrb, 13, hb ? UI::ACCENT2 : UI::TEXT2);
    UI::centerText(back, 400.f, 470.f); w.draw(back);

    if (gs.settings.scanlines) UI::drawScanlines(w);
}
