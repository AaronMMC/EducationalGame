#include "TitleScreen.h"
#include "../UIHelpers.h"
#include <cmath>

static const std::vector<std::string> MENU_LABELS = {
    "MULTIPLAYER VS",
    "SINGLE PLAYER",
    "PROFILE",
    "SETTINGS"
};

static const float BTN_W = 280.f, BTN_H = 48.f, BTN_X = 260.f;
static float btnY(int i) { return 280.f + i * 64.f; }

void TitleScreen::onEnter(GameState& gs) {
    glowPhase = 0.f;
    tickerX   = 900.f;
}

bool TitleScreen::isMouseOverBtn(const sf::Vector2i& m, int i) const {
    return m.x >= BTN_X && m.x <= BTN_X + BTN_W &&
           m.y >= btnY(i) && m.y <= btnY(i) + BTN_H;
}

void TitleScreen::handleEvent(sf::Event& event, GameState& gs) {
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2i m(event.mouseMove.x, event.mouseMove.y);
        hoveredBtn = -1;
        for (int i = 0; i < 4; ++i)
            if (isMouseOverBtn(m, i)) hoveredBtn = i;
    }
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i m(event.mouseButton.x, event.mouseButton.y);
        for (int i = 0; i < 4; ++i) {
            if (!isMouseOverBtn(m, i)) continue;
            switch (i) {
                case 0: // VS
                    gs.mode = GameMode::VS;
                    gs.currentScreen = Screen::AvatarSelect;
                    break;
                case 1: // SP
                    gs.currentScreen = Screen::SPSelect;
                    break;
                case 2:
                    gs.currentScreen = Screen::Profile;
                    break;
                case 3:
                    gs.currentScreen = Screen::Settings;
                    break;
            }
        }
    }
}

void TitleScreen::update(float dt, GameState& gs) {
    glowPhase += dt * 1.8f;
    tickerX   -= dt * 120.f;
    if (tickerX < -1400.f) tickerX = 900.f;
}

void TitleScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);
    UI::drawCornerBrackets(w, 10, 10, 780, 580, UI::ACCENT, 24.f);

    drawLogo(w, gs);
    drawMenu(w, gs);
    drawTicker(w, gs);

    if (gs.settings.scanlines) UI::drawScanlines(w);
}

void TitleScreen::drawLogo(sf::RenderWindow& w, GameState& gs) {
    float pulse = std::abs(std::sin(glowPhase));
    sf::Uint8 alpha = (sf::Uint8)(180 + 75 * pulse);

    sf::Text logo = UI::makeText("SYNTYPE", gs.fontOrb, 72, UI::ACCENT);
    logo.setStyle(sf::Text::Bold);
    UI::centerText(logo, 400.f, 140.f);
    UI::drawGlowText(w, logo, sf::Color(0, 240, 255, alpha), 6.f);

    sf::Text sub = UI::makeText("// ARCADE TYPING COMBAT  v2.0 //", gs.fontMono, 13, UI::TEXT2);
    UI::centerText(sub, 400.f, 200.f);
    w.draw(sub);
}

void TitleScreen::drawMenu(sf::RenderWindow& w, GameState& gs) {
    for (int i = 0; i < 4; ++i) {
        bool hov = (hoveredBtn == i);
        bool secondary = (i >= 2);
        sf::Color border = secondary
            ? (hov ? UI::ACCENT2 : UI::TEXT2)
            : (hov ? UI::ACCENT  : sf::Color(UI::ACCENT.r, UI::ACCENT.g, UI::ACCENT.b, 180));
        sf::Color fill = hov
            ? sf::Color(border.r, border.g, border.b, 25)
            : UI::BG2;

        UI::drawPanel(w, BTN_X, btnY(i), BTN_W, BTN_H, border, fill, 10.f);

        sf::Color txtCol = hov ? border : (secondary ? UI::TEXT2 : UI::ACCENT);
        sf::Text label = UI::makeText(MENU_LABELS[i], gs.fontOrb, 14, txtCol);
        UI::centerText(label, BTN_X + BTN_W / 2.f, btnY(i) + BTN_H / 2.f);
        w.draw(label);
    }
}

void TitleScreen::drawTicker(sf::RenderWindow& w, GameState& gs) {
    const std::string tick =
        "// SYNTYPE ARCADE // READY PLAYER ONE // TYPE TO FIGHT // "
        "COMBO MULTIPLIER ACTIVE // BOSS BATTLE AT WAVE 10 //  ";
    sf::Text t = UI::makeText(tick, gs.fontMono, 11, UI::TEXT3);
    t.setPosition(tickerX, 566.f);
    w.draw(t);
}
