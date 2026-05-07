#include "ResultScreen.h"
#include "../UIHelpers.h"
#include <cmath>

void ResultScreen::onEnter(GameState& gs) { enterAnim = 0.f; }
void ResultScreen::update(float dt, GameState& gs) {
    enterAnim = std::min(1.f, enterAnim + dt * 2.5f);
}
void ResultScreen::handleEvent(sf::Event& event, GameState& gs) {
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i m(event.mouseButton.x, event.mouseButton.y);
        if (m.x >= 300 && m.x <= 500 && m.y >= 430 && m.y <= 475)
            gs.currentScreen = Screen::Title;
    }
    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::Enter)
        gs.currentScreen = Screen::Title;
}

void ResultScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);

    sf::RectangleShape overlay({800.f, 600.f});
    overlay.setFillColor(sf::Color(10, 12, 16, 220));
    w.draw(overlay);

    float scale = 0.85f + 0.15f * enterAnim;

    bool isVS = (gs.mode == GameMode::VS);
    std::string titleStr = isVS ? (gs.playerWon ? "VICTORY" : "DEFEAT") : "RUN COMPLETE";
    sf::Color titleCol   = isVS ? (gs.playerWon ? UI::ACCENT3 : UI::ACCENT2) : UI::ACCENT;

    sf::Text title = UI::makeText(titleStr, gs.fontOrb, 36, titleCol);
    title.setStyle(sf::Text::Bold);
    title.setScale(scale, scale);
    UI::centerText(title, 400.f, 160.f);
    UI::drawGlowText(w, title, titleCol, 6.f);

    // Score
    std::string scoreStr = std::to_string(gs.score) + " PTS";
    sf::Text scoreT = UI::makeText(scoreStr, gs.fontOrb, 48, UI::ACCENT3);
    scoreT.setScale(scale, scale);
    UI::centerText(scoreT, 400.f, 230.f);
    UI::drawGlowText(w, scoreT, UI::ACCENT3, 5.f);

    // Panel
    UI::drawPanel(w, 200.f, 290.f, 400.f, 120.f, sf::Color(UI::ACCENT.r,UI::ACCENT.g,UI::ACCENT.b,80), UI::BG2, 12.f);

    auto stat = [&](float x, float y, const std::string& label, const std::string& val, sf::Color col) {
        sf::Text l = UI::makeText(label, gs.fontMono, 11, UI::TEXT2);
        l.setPosition(x, y);
        w.draw(l);
        sf::Text v = UI::makeText(val, gs.fontOrb, 20, col);
        v.setPosition(x, y + 16);
        w.draw(v);
    };

    stat(230, 302, "BEST WPM",  std::to_string(gs.stats.getWPM()),        UI::ACCENT);
    stat(340, 302, "ACCURACY",  std::to_string(gs.stats.getAccuracy())+"%",UI::GREEN);
    stat(450, 302, "ERRORS",    std::to_string(gs.stats.errors),           UI::RED);
    stat(230, 352, "MULTI",     "x"+std::to_string((int)gs.multiplier),    UI::ACCENT3);
    if (isVS) {
        stat(340, 352, "YOUR HP",  std::to_string((int)gs.playerHP),       UI::ACCENT);
        stat(450, 352, "OPP HP",   std::to_string((int)gs.oppHP),          UI::ACCENT2);
    }

    // Back button
    sf::Vector2i m = sf::Mouse::getPosition(w);
    bool hov = (m.x >= 300 && m.x <= 500 && m.y >= 430 && m.y <= 475);
    UI::drawPanel(w, 300, 430, 200, 45, hov ? UI::ACCENT : sf::Color(UI::ACCENT.r,UI::ACCENT.g,UI::ACCENT.b,140),
                  hov ? sf::Color(0,240,255,20) : UI::BG2, 8.f);
    sf::Text back = UI::makeText("BACK TO HQ", gs.fontOrb, 13, hov ? UI::ACCENT : UI::TEXT2);
    UI::centerText(back, 400.f, 452.f);
    w.draw(back);

    if (gs.settings.scanlines) UI::drawScanlines(w);
}
