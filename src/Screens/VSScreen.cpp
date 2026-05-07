#include "VSScreen.h"
#include "../UIHelpers.h"
#include <cmath>

void VSScreen::onEnter(GameState& gs) {
    animTime   = 0.f;
    phase      = 0.f;
    countTimer = 0.f;
    countdown  = 3;
    p1Ready    = false;
    p2Ready    = false;
    p1ConnectT = 0.f;
    p2ConnectT = 0.f;
}

void VSScreen::handleEvent(sf::Event& event, GameState& gs) {
    // No input needed - fully automated
}

void VSScreen::update(float dt, GameState& gs) {
    animTime += dt;

    // Phase 0: simulate P1 connecting (left side lights up)
    if (phase < 1.f) {
        p1ConnectT += dt * 0.8f;
        if (p1ConnectT >= 1.f) { p1ConnectT = 1.f; p1Ready = true; }
    }

    // P2 connects slightly after P1
    if (p1ConnectT > 0.4f) {
        p2ConnectT += dt * 0.7f;
        if (p2ConnectT >= 1.f) { p2ConnectT = 1.f; p2Ready = true; }
    }

    // Both connected - move to countdown phase
    if (p1Ready && p2Ready && phase < 1.f) {
        phase = 1.f;
    }

    // Countdown phase
    if (phase >= 1.f) {
        countTimer += dt;
        if (countTimer >= 1.f) {
            countdown--;
            countTimer = 0.f;
            if (countdown <= 0) {
                gs.resetMatch();
                gs.currentScreen = Screen::Game;
            }
        }
    }
}

void VSScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);

    auto avatars = Avatar::all();
    const Avatar& p1av = avatars[gs.profile.avatarId];
    const Avatar& p2av = avatars[(gs.profile.avatarId + 2) % 4];

    sf::Color p1Col = gs.playerAccentColor();
    sf::Color p2Col = UI::ACCENT2;

    // ── Left side (Player) ────────────────────────────────────────────────────
    float p1alpha = p1ConnectT;

    // P1 background glow
    if (p1Ready) {
        float pulse = 0.5f + 0.5f * std::sin(animTime * 4.f);
        sf::RectangleShape glow({400.f, 600.f});
        glow.setPosition(0, 0);
        glow.setFillColor({p1Col.r, p1Col.g, p1Col.b, (sf::Uint8)(20 + 15 * pulse)});
        w.draw(glow);
    }

    // P1 panel
    sf::RectangleShape p1panel({360.f, 400.f});
    p1panel.setPosition(20.f, 100.f);
    p1panel.setFillColor({p1Col.r, p1Col.g, p1Col.b, (sf::Uint8)(20 * p1alpha)});
    p1panel.setOutlineColor({p1Col.r, p1Col.g, p1Col.b, (sf::Uint8)(200 * p1alpha)});
    p1panel.setOutlineThickness(2.f);
    w.draw(p1panel);

    // P1 symbol
    sf::Text p1sym = UI::makeText(p1av.symbol, gs.fontMono, 42,
                                   {p1Col.r, p1Col.g, p1Col.b, (sf::Uint8)(255 * p1alpha)});
    UI::centerText(p1sym, 200.f, 220.f);
    if (p1Ready) UI::drawGlowText(w, p1sym, p1Col, 8.f);
    else w.draw(p1sym);

    sf::Text p1name = UI::makeText(gs.profile.name, gs.fontOrb, 20,
                                    {UI::TEXT.r, UI::TEXT.g, UI::TEXT.b, (sf::Uint8)(255 * p1alpha)});
    UI::centerText(p1name, 200.f, 280.f);
    w.draw(p1name);

    sf::Text p1av_name = UI::makeText(p1av.name + " - " + p1av.tagline, gs.fontMono, 12,
                                       {p1av.accentColor.r, p1av.accentColor.g, p1av.accentColor.b,
                                        (sf::Uint8)(255 * p1alpha)});
    UI::centerText(p1av_name, 200.f, 312.f);
    w.draw(p1av_name);

    // P1 connect status
    std::string p1status = p1Ready ? "CONNECTED" : "CONNECTING...";
    sf::Color   p1scol   = p1Ready ? UI::GREEN : UI::TEXT2;
    sf::Text p1stat = UI::makeText(p1status, gs.fontMono, 11, p1scol);
    UI::centerText(p1stat, 200.f, 370.f);
    w.draw(p1stat);

    // P1 progress bar
    UI::drawProgressBar(w, 60.f, 390.f, 280.f, 6.f, p1ConnectT, p1Col, {20, 24, 34});

    // ── Right side (CPU/Opponent) ──────────────────────────────────────────────
    float p2alpha = p2ConnectT;

    if (p2Ready) {
        float pulse = 0.5f + 0.5f * std::sin(animTime * 4.f + 1.5f);
        sf::RectangleShape glow({400.f, 600.f});
        glow.setPosition(400, 0);
        glow.setFillColor({p2Col.r, p2Col.g, p2Col.b, (sf::Uint8)(20 + 15 * pulse)});
        w.draw(glow);
    }

    sf::RectangleShape p2panel({360.f, 400.f});
    p2panel.setPosition(420.f, 100.f);
    p2panel.setFillColor({p2Col.r, p2Col.g, p2Col.b, (sf::Uint8)(20 * p2alpha)});
    p2panel.setOutlineColor({p2Col.r, p2Col.g, p2Col.b, (sf::Uint8)(200 * p2alpha)});
    p2panel.setOutlineThickness(2.f);
    w.draw(p2panel);

    sf::Text p2sym = UI::makeText(p2av.symbol, gs.fontMono, 42,
                                   {p2Col.r, p2Col.g, p2Col.b, (sf::Uint8)(255 * p2alpha)});
    UI::centerText(p2sym, 600.f, 220.f);
    if (p2Ready) UI::drawGlowText(w, p2sym, p2Col, 8.f);
    else w.draw(p2sym);

    std::string cpuName = "CPU-" + p2av.name;
    sf::Text p2name = UI::makeText(cpuName, gs.fontOrb, 20,
                                    {UI::TEXT.r, UI::TEXT.g, UI::TEXT.b, (sf::Uint8)(255 * p2alpha)});
    UI::centerText(p2name, 600.f, 280.f);
    w.draw(p2name);

    sf::Text p2av_name = UI::makeText(p2av.name + " - " + p2av.tagline, gs.fontMono, 12,
                                       {p2av.accentColor.r, p2av.accentColor.g, p2av.accentColor.b,
                                        (sf::Uint8)(255 * p2alpha)});
    UI::centerText(p2av_name, 600.f, 312.f);
    w.draw(p2av_name);

    std::string p2status = p2Ready ? "CONNECTED" : (p1Ready ? "CONNECTING..." : "WAITING...");
    sf::Color   p2scol   = p2Ready ? UI::GREEN : UI::TEXT2;
    sf::Text p2stat = UI::makeText(p2status, gs.fontMono, 11, p2scol);
    UI::centerText(p2stat, 600.f, 370.f);
    w.draw(p2stat);

    UI::drawProgressBar(w, 460.f, 390.f, 280.f, 6.f, p2ConnectT, p2Col, {20, 24, 34});

    // ── Center VS divider ─────────────────────────────────────────────────────
    sf::RectangleShape divLine({2.f, 400.f});
    divLine.setPosition(399.f, 100.f);
    divLine.setFillColor({60, 70, 90});
    w.draw(divLine);

    float vsAlpha = std::min(1.f, p1ConnectT + p2ConnectT);
    sf::Text vs = UI::makeText("VS", gs.fontOrb, 36, UI::ACCENT3);
    vs.setStyle(sf::Text::Bold);
    UI::centerText(vs, 400.f, 295.f);
    if (p1Ready && p2Ready) UI::drawGlowText(w, vs, UI::ACCENT3, 6.f);
    else w.draw(vs);

    // ── Title ─────────────────────────────────────────────────────────────────
    sf::Text title = UI::makeText("COMBAT INITIALIZED", gs.fontOrb, 18, UI::ACCENT);
    UI::centerText(title, 400.f, 40.f);
    UI::drawGlowText(w, title, UI::ACCENT, 4.f);

    // ── Countdown ─────────────────────────────────────────────────────────────
    if (phase >= 1.f) {
        std::string cdStr = countdown > 0
            ? std::to_string(countdown)
            : "FIGHT!";
        sf::Color cdCol = countdown > 0 ? UI::ACCENT3 : UI::GREEN;
        sf::Text cd = UI::makeText(cdStr, gs.fontOrb, 48, cdCol);
        cd.setStyle(sf::Text::Bold);
        float scale = 1.f + 0.3f * (1.f - countTimer);
        cd.setScale(scale, scale);
        UI::centerText(cd, 400.f, 490.f);
        UI::drawGlowText(w, cd, cdCol, 8.f);

        sf::Text cdLabel = UI::makeText("ROUND 1 BEGINS IN...", gs.fontMono, 11, UI::TEXT2);
        UI::centerText(cdLabel, 400.f, 458.f);
        w.draw(cdLabel);
    } else {
        sf::Text wait = UI::makeText("ESTABLISHING CONNECTION...", gs.fontMono, 12, UI::TEXT2);
        float alpha = 0.5f + 0.5f * std::sin(animTime * 3.f);
        wait.setFillColor({UI::TEXT2.r, UI::TEXT2.g, UI::TEXT2.b, (sf::Uint8)(alpha * 255)});
        UI::centerText(wait, 400.f, 490.f);
        w.draw(wait);
    }

    UI::drawCornerBrackets(w, 8, 8, 792, 592, UI::ACCENT, 16.f);
    if (gs.settings.scanlines) UI::drawScanlines(w);
}
