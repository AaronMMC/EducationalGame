#include "SettingsScreen.h"
#include "../UIHelpers.h"
#include <cmath>
#include <string>

void SettingsScreen::onEnter(GameState& gs) {
    dragSlider = -1;
}

void SettingsScreen::handleEvent(sf::Event& event, GameState& gs) {
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i m(event.mouseButton.x, event.mouseButton.y);

        // Back button
        if (m.x >= 300 && m.x <= 500 && m.y >= 530 && m.y <= 570)
            gs.currentScreen = Screen::Title;

        // Scanlines toggle
        if (m.x >= 400 && m.x <= 460 && m.y >= 230 && m.y <= 258)
            gs.settings.scanlines = !gs.settings.scanlines;

        // Dark mode toggle
        if (m.x >= 400 && m.x <= 460 && m.y >= 278 && m.y <= 306)
            gs.settings.darkMode = !gs.settings.darkMode;

        // Glitch effects toggle
        if (m.x >= 400 && m.x <= 460 && m.y >= 326 && m.y <= 354)
            gs.settings.glitchEffects = !gs.settings.glitchEffects;

        // Sliders
        if (m.x >= 400 && m.x <= 620 && m.y >= 138 && m.y <= 162)
            dragSlider = 0;
        if (m.x >= 400 && m.x <= 620 && m.y >= 186 && m.y <= 210)
            dragSlider = 1;
    }

    if (event.type == sf::Event::MouseButtonReleased)
        dragSlider = -1;

    if (event.type == sf::Event::MouseMoved && dragSlider >= 0) {
        float x = (float)event.mouseMove.x;
        float val = std::max(0.f, std::min(1.f, (x - 400.f) / 220.f));
        if (dragSlider == 0) gs.settings.musicVolume = val;
        if (dragSlider == 1) gs.settings.sfxVolume   = val;
    }

    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::Escape)
        gs.currentScreen = Screen::Title;
}

void SettingsScreen::update(float dt, GameState& gs) {}

void SettingsScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);
    UI::drawCornerBrackets(w, 10, 10, 780, 580, UI::ACCENT, 20.f);

    sf::Text title = UI::makeText("SYSTEM SETTINGS", gs.fontOrb, 22, UI::ACCENT);
    UI::centerText(title, 400.f, 50.f);
    UI::drawGlowText(w, title, UI::ACCENT, 4.f);

    float panelX = 120.f, panelW = 560.f;
    UI::drawPanel(w, panelX, 100.f, panelW, 400.f,
                  sf::Color(UI::ACCENT.r, UI::ACCENT.g, UI::ACCENT.b, 60), UI::BG2, 8.f);

    // ── Sliders ───────────────────────────────────────────────────────────────
    auto drawSlider = [&](const std::string& label, float val, float y) {
        sf::Text lbl = UI::makeText(label, gs.fontMono, 11, UI::TEXT2);
        lbl.setPosition(140.f, y);
        w.draw(lbl);

        // Track
        sf::RectangleShape track({220.f, 6.f});
        track.setPosition(400.f, y + 8.f);
        track.setFillColor(UI::BG3);
        track.setOutlineColor(UI::TEXT3);
        track.setOutlineThickness(1.f);
        w.draw(track);

        // Fill
        sf::RectangleShape fill({220.f * val, 6.f});
        fill.setPosition(400.f, y + 8.f);
        fill.setFillColor(UI::ACCENT);
        w.draw(fill);

        // Knob
        sf::CircleShape knob(7.f);
        knob.setPosition(400.f + 220.f * val - 7.f, y + 1.f);
        knob.setFillColor(UI::ACCENT);
        knob.setOutlineColor(UI::TEXT);
        knob.setOutlineThickness(1.f);
        w.draw(knob);

        // Value label
        int pct = (int)(val * 100.f);
        sf::Text valT = UI::makeText(std::to_string(pct) + "%", gs.fontMono, 10, UI::ACCENT);
        valT.setPosition(630.f, y + 4.f);
        w.draw(valT);
    };

    drawSlider("MUSIC VOLUME", gs.settings.musicVolume, 138.f);
    drawSlider("SFX VOLUME",   gs.settings.sfxVolume,   186.f);

    // ── Toggles ───────────────────────────────────────────────────────────────
    auto drawToggle = [&](const std::string& label, bool val, float y) {
        sf::Text lbl = UI::makeText(label, gs.fontMono, 11, UI::TEXT2);
        lbl.setPosition(140.f, y + 5.f);
        w.draw(lbl);

        sf::RectangleShape box({60.f, 28.f});
        box.setPosition(400.f, y);
        box.setFillColor(val
            ? sf::Color(UI::ACCENT.r / 5, UI::ACCENT.g / 5, UI::ACCENT.b / 5)
            : UI::BG3);
        box.setOutlineColor(val ? UI::ACCENT : UI::TEXT3);
        box.setOutlineThickness(1.f);
        w.draw(box);

        sf::Text valT = UI::makeText(val ? "ON" : "OFF", gs.fontOrb, 11,
                                      val ? UI::ACCENT : UI::TEXT3);
        UI::centerText(valT, 430.f, y + 14.f);
        w.draw(valT);
    };

    drawToggle("SCANLINES",     gs.settings.scanlines,     230.f);
    drawToggle("DARK MODE",     gs.settings.darkMode,      278.f);
    drawToggle("GLITCH EFFECTS",gs.settings.glitchEffects, 326.f);

    // Time limit display
    sf::Text timeLbl = UI::makeText("TIME LIMIT (VS)", gs.fontMono, 11, UI::TEXT2);
    timeLbl.setPosition(140.f, 380.f);
    w.draw(timeLbl);
    sf::Text timeVal = UI::makeText(std::to_string((int)gs.settings.timeLimit) + "s",
                                     gs.fontOrb, 14, UI::ACCENT3);
    timeVal.setPosition(400.f, 376.f);
    w.draw(timeVal);

    // Increase / decrease time
    sf::Vector2i mouse = sf::Mouse::getPosition(w);
    bool hMinus = (mouse.x >= 440 && mouse.x <= 480 && mouse.y >= 374 && mouse.y <= 400);
    bool hPlus  = (mouse.x >= 490 && mouse.x <= 530 && mouse.y >= 374 && mouse.y <= 400);

    sf::RectangleShape minusBtn({36.f, 26.f});
    minusBtn.setPosition(440.f, 374.f);
    minusBtn.setFillColor(hMinus ? UI::BG3 : UI::BG);
    minusBtn.setOutlineColor(UI::TEXT3);
    minusBtn.setOutlineThickness(1.f);
    w.draw(minusBtn);
    sf::Text minus = UI::makeText("-", gs.fontOrb, 14, UI::TEXT2);
    UI::centerText(minus, 458.f, 387.f);
    w.draw(minus);

    sf::RectangleShape plusBtn({36.f, 26.f});
    plusBtn.setPosition(490.f, 374.f);
    plusBtn.setFillColor(hPlus ? UI::BG3 : UI::BG);
    plusBtn.setOutlineColor(UI::TEXT3);
    plusBtn.setOutlineThickness(1.f);
    w.draw(plusBtn);
    sf::Text plus = UI::makeText("+", gs.fontOrb, 14, UI::TEXT2);
    UI::centerText(plus, 508.f, 387.f);
    w.draw(plus);

    // Back
    bool hovBack = (mouse.x >= 300 && mouse.x <= 500 && mouse.y >= 530 && mouse.y <= 570);
    UI::drawPanel(w, 300, 530, 200, 40,
                  hovBack ? UI::ACCENT2 : UI::TEXT2,
                  hovBack ? sf::Color(255, 60, 110, 20) : UI::BG2, 8.f);
    sf::Text back = UI::makeText("BACK TO HQ", gs.fontOrb, 13, hovBack ? UI::ACCENT2 : UI::TEXT2);
    UI::centerText(back, 400.f, 550.f);
    w.draw(back);

    if (gs.settings.scanlines) UI::drawScanlines(w);
}
