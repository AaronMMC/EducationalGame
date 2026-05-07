#include "SettingsScreen.h"
#include "../UIHelpers.h"
#include <cmath>

struct SettingRow { std::string label; std::string desc; bool isToggle; float* floatVal; bool* boolVal; float minV, maxV, step; };

static std::vector<SettingRow> buildRows(AppSettings& s) {
    return {
        {"MUSIC VOLUME",   "Background soundtrack intensity", false, &s.musicVolume, nullptr, 0.f,1.f,0.05f},
        {"SFX VOLUME",     "Key clicks, combos and hit sounds", false, &s.sfxVolume, nullptr, 0.f,1.f,0.05f},
        {"TIME LIMIT",     "Per-round timer (seconds)", false, &s.timeLimit, nullptr, 15.f,120.f,15.f},
        {"SCANLINE FX",    "CRT scanline visual overlay", true, nullptr, &s.scanlines, 0,0,0},
        {"DARK MODE",      "Pure dark background (recommended)", true, nullptr, &s.darkMode, 0,0,0},
        {"GLITCH EFFECTS", "Character glitch on errors", true, nullptr, &s.glitchEffects, 0,0,0},
    };
}

void SettingsScreen::handleEvent(sf::Event& event, GameState& gs) {
    auto rows = buildRows(gs.settings);
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i m(event.mouseButton.x, event.mouseButton.y);
        if (m.x >= 300 && m.x <= 500 && m.y >= 530 && m.y <= 570)
            gs.currentScreen = Screen::Title;
        for (int i = 0; i < (int)rows.size(); ++i) {
            float ry = 100.f + i * 64.f;
            auto& r = rows[i];
            if (r.isToggle && m.x >= 660 && m.x <= 710 && m.y >= ry+8 && m.y <= ry+32) {
                *r.boolVal = !*r.boolVal;
            }
            if (!r.isToggle) {
                // dec / inc buttons
                if (m.x >= 580 && m.x <= 610 && m.y >= ry+6 && m.y <= ry+30)
                    *r.floatVal = std::max(r.minV, *r.floatVal - r.step);
                if (m.x >= 680 && m.x <= 710 && m.y >= ry+6 && m.y <= ry+30)
                    *r.floatVal = std::min(r.maxV, *r.floatVal + r.step);
            }
        }
    }
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2i m(event.mouseMove.x, event.mouseMove.y);
        hoveredRow = -1;
        for (int i = 0; i < (int)rows.size(); ++i) {
            float ry = 100.f + i * 64.f;
            if (m.y >= ry && m.y <= ry + 50) hoveredRow = i;
        }
    }
    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::Escape)
        gs.currentScreen = Screen::Title;
}

void SettingsScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);
    UI::drawCornerBrackets(w, 10, 10, 780, 580, UI::ACCENT, 20.f);

    sf::Text title = UI::makeText("SYSTEM CONFIG", gs.fontOrb, 22, UI::ACCENT);
    UI::centerText(title, 400.f, 50.f);
    UI::drawGlowText(w, title, UI::ACCENT, 4.f);
    sf::Text sub = UI::makeText("ADJUST PARAMETERS", gs.fontMono, 11, UI::TEXT2);
    UI::centerText(sub, 400.f, 76.f);
    w.draw(sub);

    auto rows = buildRows(gs.settings);
    for (int i = 0; i < (int)rows.size(); ++i) {
        float ry = 100.f + i * 64.f;
        bool hov = (hoveredRow == i);
        sf::RectangleShape rowBg({720.f, 50.f});
        rowBg.setPosition(40.f, ry);
        rowBg.setFillColor(hov ? sf::Color(UI::ACCENT.r,UI::ACCENT.g,UI::ACCENT.b,8) : UI::BG2);
        rowBg.setOutlineColor(sf::Color(UI::ACCENT.r,UI::ACCENT.g,UI::ACCENT.b,20));
        rowBg.setOutlineThickness(1.f);
        w.draw(rowBg);

        sf::Text lbl = UI::makeText(rows[i].label, gs.fontRaj, 14, UI::TEXT);
        lbl.setStyle(sf::Text::Bold);
        lbl.setPosition(56, ry + 8); w.draw(lbl);
        sf::Text desc = UI::makeText(rows[i].desc, gs.fontMono, 10, UI::TEXT2);
        desc.setPosition(56, ry + 28); w.draw(desc);

        const auto& r = rows[i];
        if (r.isToggle) {
            bool on = *r.boolVal;
            sf::RectangleShape track({44.f, 22.f});
            track.setPosition(664, ry + 14);
            track.setFillColor(on ? sf::Color(0,240,255,30) : UI::BG3);
            track.setOutlineColor(on ? UI::ACCENT : UI::TEXT3);
            track.setOutlineThickness(1.f);
            w.draw(track);
            sf::CircleShape thumb(8.f);
            thumb.setPosition(on ? 690.f : 668.f, ry + 17.f);
            thumb.setFillColor(on ? UI::ACCENT : UI::TEXT3);
            w.draw(thumb);
        } else {
            // Minus button
            sf::Text minus = UI::makeText("-", gs.fontOrb, 16, UI::ACCENT);
            minus.setPosition(588, ry + 10); w.draw(minus);
            // Value
            std::string valStr;
            if (r.floatVal == &gs.settings.timeLimit)
                valStr = std::to_string((int)*r.floatVal) + "s";
            else
                valStr = std::to_string((int)(*r.floatVal * 100)) + "%";
            sf::Text val = UI::makeText(valStr, gs.fontOrb, 14, UI::ACCENT3);
            UI::centerText(val, 645.f, ry + 22.f); w.draw(val);
            // Plus button
            sf::Text plus = UI::makeText("+", gs.fontOrb, 16, UI::ACCENT);
            plus.setPosition(688, ry + 10); w.draw(plus);
        }
    }

    sf::Vector2i m = sf::Mouse::getPosition(w);
    bool hb = (m.x >= 300 && m.x <= 500 && m.y >= 530 && m.y <= 570);
    UI::drawPanel(w, 300, 530, 200, 40, hb ? UI::ACCENT2 : UI::TEXT2,
                  hb ? sf::Color(255,60,110,20) : UI::BG2, 8.f);
    sf::Text back = UI::makeText("BACK TO HQ", gs.fontOrb, 13, hb ? UI::ACCENT2 : UI::TEXT2);
    UI::centerText(back, 400.f, 550.f); w.draw(back);

    if (gs.settings.scanlines) UI::drawScanlines(w);
}
