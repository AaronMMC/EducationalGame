#include "VSScreen.h"
#include "../UIHelpers.h"
#include <cmath>

void VSScreen::onEnter(GameState& gs) {
    timer = 0.f; barFill = 0.f; countdown = 3;
    p1Lit = false; p2Lit = false; started = false; vsPulse = 0.f;
    gs.resetMatch();
}

void VSScreen::handleEvent(sf::Event& event, GameState& gs) {}

void VSScreen::update(float dt, GameState& gs) {
    timer    += dt;
    vsPulse  += dt * 2.f;

    if (!p1Lit && timer > 0.5f)  p1Lit = true;
    if (!p2Lit && timer > 1.3f)  p2Lit = true;

    barFill = std::min(1.f, (timer - 1.5f) / 3.f);

    if (!started && timer > 1.5f) {
        float cd = timer - 1.5f;
        countdown = std::max(0, 3 - (int)cd);
        if (cd >= 3.5f) {
            started = true;
            gs.currentScreen = Screen::Game;
        }
    }
}

void VSScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);
    UI::drawCornerBrackets(w, 10, 10, 780, 580, UI::ACCENT2, 20.f);

    auto avatars = Avatar::all();
    const Avatar& p1av = avatars[gs.profile.avatarId];
    const Avatar& p2av = avatars[(gs.profile.avatarId + 2) % 4];

    // P1 side
    float p1Alpha = p1Lit ? 255.f : 0.f;
    float p1X = 180.f;
    sf::Color p1Color = sf::Color(UI::ACCENT.r, UI::ACCENT.g, UI::ACCENT.b, (sf::Uint8)p1Alpha);
    sf::Color p1Fill  = sf::Color(0, 240, 255, (sf::Uint8)(p1Lit ? 15 : 0));
    if (p1Lit) {
        UI::drawPanel(w, p1X - 70, 180, 140, 140, p1Color, p1Fill, 16.f);
        sf::Text sym = UI::makeText(p1av.symbol, gs.fontMono, 24, p1Color);
        UI::centerText(sym, p1X, 250.f);
        UI::drawGlowText(w, sym, UI::ACCENT, 6.f);

        sf::Text label = UI::makeText("PLAYER 1", gs.fontOrb, 11, UI::ACCENT);
        UI::centerText(label, p1X, 340.f);
        w.draw(label);

        sf::Text name = UI::makeText(gs.profile.name, gs.fontOrb, 18, UI::TEXT);
        name.setStyle(sf::Text::Bold);
        UI::centerText(name, p1X, 365.f);
        w.draw(name);

        sf::Text avName = UI::makeText(p1av.name, gs.fontMono, 12, p1av.accentColor);
        UI::centerText(avName, p1X, 390.f);
        w.draw(avName);
    }

    // P2 side
    float p2Alpha = p2Lit ? 255.f : 0.f;
    float p2X = 620.f;
    sf::Color p2Color = sf::Color(UI::ACCENT2.r, UI::ACCENT2.g, UI::ACCENT2.b, (sf::Uint8)p2Alpha);
    sf::Color p2Fill  = sf::Color(255, 60, 110, (sf::Uint8)(p2Lit ? 15 : 0));
    if (p2Lit) {
        UI::drawPanel(w, p2X - 70, 180, 140, 140, p2Color, p2Fill, 16.f);
        sf::Text sym = UI::makeText(p2av.symbol, gs.fontMono, 24, p2Color);
        UI::centerText(sym, p2X, 250.f);
        UI::drawGlowText(w, sym, UI::ACCENT2, 6.f);

        sf::Text label = UI::makeText("PLAYER 2", gs.fontOrb, 11, UI::ACCENT2);
        UI::centerText(label, p2X, 340.f);
        w.draw(label);

        std::string oppName = "CPU-" + p2av.name;
        sf::Text name = UI::makeText(oppName, gs.fontOrb, 18, UI::TEXT);
        name.setStyle(sf::Text::Bold);
        UI::centerText(name, p2X, 365.f);
        w.draw(name);

        sf::Text avName = UI::makeText(p2av.name, gs.fontMono, 12, p2av.accentColor);
        UI::centerText(avName, p2X, 390.f);
        w.draw(avName);
    }

    // VS text
    float pulse = 0.95f + 0.05f * std::sin(vsPulse);
    sf::Text vs = UI::makeText("VS", gs.fontOrb, 52, UI::ACCENT2);
    vs.setStyle(sf::Text::Bold);
    vs.setScale(pulse, pulse);
    UI::centerText(vs, 400.f, 250.f);
    UI::drawGlowText(w, vs, UI::ACCENT2, 6.f);

    // Status / countdown
    std::string statusStr;
    if (!p1Lit) statusStr = "ESTABLISHING LINK...";
    else if (!p2Lit) statusStr = "PLAYER 1 CONNECTED";
    else if (countdown > 0) statusStr = "MATCH STARTS IN " + std::to_string(countdown) + "...";
    else statusStr = "FIGHT!";

    sf::Text status = UI::makeText(statusStr, gs.fontMono, 13, UI::TEXT2);
    UI::centerText(status, 400.f, 320.f);
    w.draw(status);

    // Connection bar
    if (barFill > 0.f) {
        UI::drawProgressBar(w, 300.f, 350.f, 200.f, 3.f, barFill, UI::ACCENT, UI::BG3);
    }

    // Bottom status line
    sf::Text bottom = UI::makeText(p2Lit ? "BOTH PLAYERS CONNECTED - MATCH READY" : "CONNECTING...",
                                   gs.fontMono, 11, UI::TEXT3);
    UI::centerText(bottom, 400.f, 560.f);
    w.draw(bottom);

    if (gs.settings.scanlines) UI::drawScanlines(w);
}
