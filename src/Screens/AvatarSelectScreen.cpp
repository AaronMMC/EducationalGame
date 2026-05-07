#include "AvatarSelectScreen.h"
#include "../UIHelpers.h"
#include <cmath>

void AvatarSelectScreen::onEnter(GameState& gs) {
    animTime = 0.f;
    hovered  = -1;
}

void AvatarSelectScreen::handleEvent(sf::Event& event, GameState& gs) {
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i m(event.mouseButton.x, event.mouseButton.y);

        auto avatars = Avatar::all();
        for (int i = 0; i < (int)avatars.size(); ++i) {
            float cx = 100.f + i * 160.f;
            if (m.x >= cx && m.x <= cx + 120 && m.y >= 160 && m.y <= 390) {
                gs.profile.avatarId = i;
            }
        }

        // Confirm button
        if (m.x >= 270 && m.x <= 530 && m.y >= 500 && m.y <= 545) {
            gs.resetMatch();
            gs.currentScreen = (gs.mode == GameMode::VS) ? Screen::VS : Screen::Game;
        }

        // Back
        if (m.x >= 10 && m.x <= 100 && m.y >= 560 && m.y <= 590) {
            gs.currentScreen = Screen::Title;
        }
    }

    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2i m(event.mouseMove.x, event.mouseMove.y);
        hovered = -1;
        auto avatars = Avatar::all();
        for (int i = 0; i < (int)avatars.size(); ++i) {
            float cx = 100.f + i * 160.f;
            if (m.x >= cx && m.x <= cx + 120 && m.y >= 160 && m.y <= 390)
                hovered = i;
        }
    }

    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::Escape)
        gs.currentScreen = Screen::Title;
}

void AvatarSelectScreen::update(float dt, GameState& gs) {
    animTime += dt;
}

void AvatarSelectScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);
    UI::drawCornerBrackets(w, 8, 8, 792, 592, UI::ACCENT, 16.f);

    sf::Text title = UI::makeText("SELECT UNIT", gs.fontOrb, 26, UI::ACCENT);
    UI::centerText(title, 400.f, 60.f);
    UI::drawGlowText(w, title, UI::ACCENT, 5.f);

    sf::Text modeLbl = UI::makeText(
        gs.mode == GameMode::VS ? "-- MULTIPLAYER VS --" : "-- SINGLE PLAYER --",
        gs.fontMono, 11, UI::TEXT2);
    UI::centerText(modeLbl, 400.f, 92.f);
    w.draw(modeLbl);

    auto avatars = Avatar::all();
    for (int i = 0; i < (int)avatars.size(); ++i) {
        const Avatar& av = avatars[i];
        float cx = 100.f + i * 160.f;
        bool  sel = (gs.profile.avatarId == i);
        bool  hov = (hovered == i);

        float pulse = sel ? (0.85f + 0.15f * std::sin(animTime * 3.f)) : 1.f;
        sf::Color col = av.accentColor;

        // Card background
        sf::RectangleShape card({120.f, 230.f});
        card.setPosition(cx, 160.f);
        card.setFillColor({(sf::Uint8)(col.r / (sel ? 8 : 14)),
                           (sf::Uint8)(col.g / (sel ? 8 : 14)),
                           (sf::Uint8)(col.b / (sel ? 8 : 14))});
        card.setOutlineColor(sel ? col : sf::Color(col.r, col.g, col.b, hov ? 160 : 60));
        card.setOutlineThickness(sel ? 2.f : 1.f);
        w.draw(card);

        // Avatar symbol
        sf::Text sym = UI::makeText(av.symbol, gs.fontMono, 18, col);
        sym.setScale(pulse, pulse);
        UI::centerText(sym, cx + 60.f, 195.f);
        if (sel) UI::drawGlowText(w, sym, col, 5.f);
        else w.draw(sym);

        // Name
        sf::Text nm = UI::makeText(av.name, gs.fontOrb, 13, sel ? UI::TEXT : UI::TEXT2);
        UI::centerText(nm, cx + 60.f, 235.f);
        w.draw(nm);

        // Tagline
        sf::Text tag = UI::makeText(av.tagline, gs.fontMono, 9, col);
        UI::centerText(tag, cx + 60.f, 258.f);
        w.draw(tag);

        // Separator
        sf::RectangleShape sep({100.f, 1.f});
        sep.setPosition(cx + 10.f, 272.f);
        sep.setFillColor({col.r, col.g, col.b, 80});
        w.draw(sep);

        // Stats bars
        auto drawStatBar = [&](const std::string& lbl, float val, float y) {
            sf::Text sl = UI::makeText(lbl, gs.fontMono, 8, UI::TEXT3);
            sl.setPosition(cx + 8.f, y);
            w.draw(sl);
            UI::drawProgressBar(w, cx + 8.f, y + 12.f, 104.f, 4.f, val, col,
                                 {20, 24, 34});
        };

        float gainNorm  = (av.multiGainRate - 0.08f) / (0.25f - 0.08f);
        float threshNorm = (av.accuracyThreshold - 75.f) / (90.f - 75.f);
        float maxNorm   = (av.maxMultiplier - 4.f) / (6.f - 4.f);

        drawStatBar("MULTI GAIN",  gainNorm,         280.f);
        drawStatBar("THRESHOLD",   threshNorm,       304.f);
        drawStatBar("MAX MULTI",   maxNorm,          328.f);

        // Selected indicator
        if (sel) {
            sf::Text selLbl = UI::makeText("SELECTED", gs.fontMono, 9, col);
            UI::centerText(selLbl, cx + 60.f, 362.f);
            w.draw(selLbl);
        }
    }

    // Ability description box for selected avatar
    const Avatar& chosen = avatars[gs.profile.avatarId];
    UI::drawPanel(w, 50.f, 400.f, 700.f, 80.f,
                  sf::Color(chosen.accentColor.r, chosen.accentColor.g, chosen.accentColor.b, 100),
                  UI::BG2);

    std::string abilityDesc;
    switch (chosen.ability) {
        case AvatarAbility::Comeback:
            abilityDesc = "PHOENIX - COMEBACK: Round 3 forces multiplier to minimum x3. Weak early, lethal late."; break;
        case AvatarAbility::Volatile:
            abilityDesc = "BLAZE - VOLATILE: Gains multiplier fastest, but full reset if accuracy drops below 90%."; break;
        case AvatarAbility::Saboteur:
            abilityDesc = "VIRUS - SABOTEUR: Every 2 seconds, scrambles 5 random characters on the opponent's screen."; break;
        case AvatarAbility::Starter:
            abilityDesc = "SENTINEL - HEAD START: Opponent cannot type for the first 4 seconds of each round."; break;
    }

    sf::Text desc = UI::makeText(abilityDesc, gs.fontMono, 11, UI::TEXT);
    UI::centerText(desc, 400.f, 440.f);
    w.draw(desc);

    // Confirm
    sf::Vector2i mouse = sf::Mouse::getPosition(w);
    bool hovConfirm = (mouse.x >= 270 && mouse.x <= 530 && mouse.y >= 500 && mouse.y <= 545);
    sf::RectangleShape confirmBg({260.f, 45.f});
    confirmBg.setPosition(270.f, 500.f);
    confirmBg.setFillColor({0, 30, 34});
    confirmBg.setOutlineColor(hovConfirm ? UI::ACCENT : sf::Color(0, 240, 255, 80));
    confirmBg.setOutlineThickness(1.f);
    w.draw(confirmBg);

    sf::Text confirm = UI::makeText("DEPLOY UNIT  >>", gs.fontOrb, 15,
                                     hovConfirm ? UI::ACCENT : UI::TEXT2);
    UI::centerText(confirm, 400.f, 522.f);
    if (hovConfirm) UI::drawGlowText(w, confirm, UI::ACCENT, 3.f);
    else w.draw(confirm);

    sf::Text back = UI::makeText("< BACK", gs.fontMono, 10, UI::TEXT3);
    back.setPosition(14.f, 568.f);
    w.draw(back);

    if (gs.settings.scanlines) UI::drawScanlines(w);
}
