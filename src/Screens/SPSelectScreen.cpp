#include "SPSelectScreen.h"
#include "../UIHelpers.h"
#include <cmath>

void SPSelectScreen::onEnter(GameState& gs) {
    animTime = 0.f;
}

void SPSelectScreen::handleEvent(sf::Event& event, GameState& gs) {
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i m(event.mouseButton.x, event.mouseButton.y);

        // Score Attack
        if (m.x >= 100 && m.x <= 380 && m.y >= 200 && m.y <= 400) {
            gs.mode = GameMode::ScoreAttack;
            gs.wave = 1;
            gs.currentScreen = Screen::AvatarSelect;
        }

        // Endless
        if (m.x >= 420 && m.x <= 700 && m.y >= 200 && m.y <= 400) {
            gs.mode = GameMode::Endless;
            gs.wave = 1;
            gs.currentScreen = Screen::AvatarSelect;
        }

        // Back
        if (m.x >= 10 && m.x <= 100 && m.y >= 560 && m.y <= 590)
            gs.currentScreen = Screen::Title;
    }

    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::Escape)
        gs.currentScreen = Screen::Title;
}

void SPSelectScreen::update(float dt, GameState& gs) {
    animTime += dt;
}

void SPSelectScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);
    UI::drawCornerBrackets(w, 8, 8, 792, 592, UI::ACCENT, 16.f);

    sf::Text title = UI::makeText("SINGLE PLAYER", gs.fontOrb, 26, UI::ACCENT);
    UI::centerText(title, 400.f, 60.f);
    UI::drawGlowText(w, title, UI::ACCENT, 5.f);

    sf::Text sub = UI::makeText("SELECT GAME MODE", gs.fontMono, 12, UI::TEXT2);
    UI::centerText(sub, 400.f, 98.f);
    w.draw(sub);

    sf::Vector2i mouse = sf::Mouse::getPosition(w);

    // ── Score Attack card ─────────────────────────────────────────────────────
    bool hovSA = (mouse.x >= 100 && mouse.x <= 380 && mouse.y >= 200 && mouse.y <= 400);
    sf::RectangleShape saCard({280.f, 200.f});
    saCard.setPosition(100.f, 200.f);
    saCard.setFillColor(hovSA
        ? sf::Color(0, 30, 34)
        : sf::Color(8, 12, 18));
    saCard.setOutlineColor(hovSA ? UI::ACCENT : sf::Color(UI::ACCENT.r, UI::ACCENT.g, UI::ACCENT.b, 80));
    saCard.setOutlineThickness(hovSA ? 2.f : 1.f);
    w.draw(saCard);

    sf::Text saTitle = UI::makeText("SCORE ATTACK", gs.fontOrb, 18, hovSA ? UI::ACCENT : UI::TEXT2);
    UI::centerText(saTitle, 240.f, 248.f);
    if (hovSA) UI::drawGlowText(w, saTitle, UI::ACCENT, 4.f);
    else w.draw(saTitle);

    sf::Text saSymbol = UI::makeText("[SA]", gs.fontMono, 32, UI::ACCENT);
    UI::centerText(saSymbol, 240.f, 300.f);
    w.draw(saSymbol);

    sf::Text saDesc = UI::makeText("One text. Highest\nscore possible.\nClean run.", gs.fontMono, 11, UI::TEXT2);
    UI::centerText(saDesc, 240.f, 358.f);
    w.draw(saDesc);

    // ── Endless card ──────────────────────────────────────────────────────────
    bool hovEN = (mouse.x >= 420 && mouse.x <= 700 && mouse.y >= 200 && mouse.y <= 400);
    sf::RectangleShape enCard({280.f, 200.f});
    enCard.setPosition(420.f, 200.f);
    enCard.setFillColor(hovEN
        ? sf::Color(34, 0, 10)
        : sf::Color(18, 8, 12));
    enCard.setOutlineColor(hovEN ? UI::ACCENT2 : sf::Color(UI::ACCENT2.r, UI::ACCENT2.g, UI::ACCENT2.b, 80));
    enCard.setOutlineThickness(hovEN ? 2.f : 1.f);
    w.draw(enCard);

    sf::Text enTitle = UI::makeText("ENDLESS SIEGE", gs.fontOrb, 18, hovEN ? UI::ACCENT2 : UI::TEXT2);
    UI::centerText(enTitle, 560.f, 248.f);
    if (hovEN) UI::drawGlowText(w, enTitle, UI::ACCENT2, 4.f);
    else w.draw(enTitle);

    sf::Text enSymbol = UI::makeText("[EN]", gs.fontMono, 32, UI::ACCENT2);
    UI::centerText(enSymbol, 560.f, 300.f);
    w.draw(enSymbol);

    sf::Text enDesc = UI::makeText("Infinite waves.\nBoss every 10.\nSurvive as long\nas possible.", gs.fontMono, 11, UI::TEXT2);
    UI::centerText(enDesc, 560.f, 354.f);
    w.draw(enDesc);

    // Boss wave info strip
    UI::drawPanel(w, 100.f, 430.f, 600.f, 60.f,
                  sf::Color(UI::ACCENT3.r, UI::ACCENT3.g, UI::ACCENT3.b, 60), UI::BG2, 6.f);
    float pulse = 0.6f + 0.4f * std::sin(animTime * 2.f);
    sf::Text bossInfo = UI::makeText("BOSS WAVES: Harder text + all avatars attacking you simultaneously",
                                      gs.fontMono, 10, {(sf::Uint8)(UI::ACCENT3.r * pulse),
                                                         (sf::Uint8)(UI::ACCENT3.g * pulse),
                                                         (sf::Uint8)(UI::ACCENT3.b * pulse)});
    UI::centerText(bossInfo, 400.f, 460.f);
    w.draw(bossInfo);

    sf::Text back = UI::makeText("< BACK", gs.fontMono, 10, UI::TEXT3);
    back.setPosition(14.f, 568.f);
    w.draw(back);

    if (gs.settings.scanlines) UI::drawScanlines(w);
}
