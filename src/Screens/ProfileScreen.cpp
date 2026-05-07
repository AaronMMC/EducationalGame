#include "ProfileScreen.h"
#include "../UIHelpers.h"

static const sf::Color COLORS[] = {
    {0,240,255},{255,60,110},{240,192,0},{123,47,255},{0,230,118},{255,145,0}
};

void ProfileScreen::onEnter(GameState& gs) {
    nameBuffer  = gs.profile.name;
    nameFocused = false;
}

void ProfileScreen::handleEvent(sf::Event& event, GameState& gs) {
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i m(event.mouseButton.x, event.mouseButton.y);
        // Back button
        if (m.x >= 300 && m.x <= 500 && m.y >= 530 && m.y <= 570)
            gs.currentScreen = Screen::Title;
        // Name field
        nameFocused = (m.x >= 40 && m.x <= 280 && m.y >= 330 && m.y <= 355);
        // Color dots
        for (int i = 0; i < 6; ++i) {
            float cx = 40.f + i * 34.f;
            if (m.x >= cx && m.x <= cx + 24 && m.y >= 380 && m.y <= 404)
                gs.profile.colorIndex = i;
        }
        // Avatar unit buttons
        for (int i = 0; i < 4; ++i) {
            float bx = 40.f + i * 56.f;
            if (m.x >= bx && m.x <= bx + 46 && m.y >= 430 && m.y <= 472)
                gs.profile.avatarId = i;
        }
    }
    if (nameFocused) {
        if (event.type == sf::Event::TextEntered) {
            sf::Uint32 u = event.text.unicode;
            if (u == 8 && !nameBuffer.empty()) nameBuffer.pop_back();
            else if (u >= 32 && u < 127 && nameBuffer.size() < 14)
                nameBuffer += (char)std::toupper((char)u);
            gs.profile.name = nameBuffer.empty() ? "UNIT ALPHA" : nameBuffer;
        }
    }
    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::Escape)
        gs.currentScreen = Screen::Title;
}

void ProfileScreen::update(float dt, GameState& gs) {}

void ProfileScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);
    UI::drawCornerBrackets(w, 10, 10, 780, 580, UI::ACCENT, 20.f);

    sf::Text title = UI::makeText("PILOT PROFILE", gs.fontOrb, 22, UI::ACCENT);
    UI::centerText(title, 400.f, 50.f);
    UI::drawGlowText(w, title, UI::ACCENT, 4.f);

    auto avatars = Avatar::all();
    const Avatar& av = avatars[gs.profile.avatarId];
    sf::Color playerCol = COLORS[gs.profile.colorIndex % 6];

    // LEFT panel — identity
    UI::drawPanel(w, 30, 90, 280, 410, sf::Color(playerCol.r,playerCol.g,playerCol.b,80), UI::BG2, 10.f);

    // Avatar display
    sf::Text sym = UI::makeText(av.symbol, gs.fontMono, 36, playerCol);
    UI::centerText(sym, 170.f, 150.f);
    UI::drawGlowText(w, sym, playerCol, 5.f);

    sf::Text dname = UI::makeText(gs.profile.name, gs.fontOrb, 18, UI::TEXT);
    UI::centerText(dname, 170.f, 195.f);
    w.draw(dname);
    sf::Text avname = UI::makeText(av.name + " - " + av.tagline, gs.fontMono, 11, av.accentColor);
    UI::centerText(avname, 170.f, 220.f);
    w.draw(avname);

    // Name input
    sf::Text flbl = UI::makeText("CALLSIGN", gs.fontMono, 10, UI::TEXT2);
    flbl.setPosition(40, 252); w.draw(flbl);
    sf::RectangleShape field({220.f, 28.f});
    field.setPosition(40, 268);
    field.setFillColor(UI::BG3);
    field.setOutlineColor(nameFocused ? UI::ACCENT : UI::TEXT3);
    field.setOutlineThickness(1.f);
    w.draw(field);
    std::string display = nameBuffer + (nameFocused ? "|" : "");
    sf::Text fnm = UI::makeText(display, gs.fontMono, 13, UI::TEXT);
    fnm.setPosition(46, 272); w.draw(fnm);

    // Color picker
    sf::Text clbl = UI::makeText("ACCENT COLOR", gs.fontMono, 10, UI::TEXT2);
    clbl.setPosition(40, 308); w.draw(clbl);
    for (int i = 0; i < 6; ++i) {
        sf::CircleShape dot(10.f);
        dot.setPosition(40.f + i * 34.f, 322.f);
        dot.setFillColor(COLORS[i]);
        if (i == gs.profile.colorIndex) {
            dot.setOutlineColor(sf::Color::White);
            dot.setOutlineThickness(2.f);
        }
        w.draw(dot);
    }

    // Unit select
    sf::Text ulbl = UI::makeText("ACTIVE UNIT", gs.fontMono, 10, UI::TEXT2);
    ulbl.setPosition(40, 360); w.draw(ulbl);
    for (int i = 0; i < 4; ++i) {
        float bx = 40.f + i * 56.f;
        bool sel = (gs.profile.avatarId == i);
        sf::RectangleShape btn({46.f, 40.f});
        btn.setPosition(bx, 374);
        btn.setFillColor(sel ? sf::Color(avatars[i].accentColor.r,avatars[i].accentColor.g,avatars[i].accentColor.b,30) : UI::BG3);
        btn.setOutlineColor(sel ? avatars[i].accentColor : UI::TEXT3);
        btn.setOutlineThickness(1.f);
        w.draw(btn);
        sf::Text bsym = UI::makeText(avatars[i].symbol, gs.fontMono, 11, avatars[i].accentColor);
        UI::centerText(bsym, bx + 23.f, 394.f);
        w.draw(bsym);
    }

    // RIGHT panels — stats
    // Combat record
    UI::drawPanel(w, 330, 90, 220, 180, sf::Color(UI::ACCENT.r,UI::ACCENT.g,UI::ACCENT.b,50), UI::BG2, 8.f);
    sf::Text rlbl = UI::makeText("COMBAT RECORD", gs.fontOrb, 9, UI::TEXT2);
    rlbl.setPosition(342, 98); w.draw(rlbl);
    float ry = 116.f;
    auto statRow = [&](const std::string& lbl, const std::string& val, sf::Color col) {
        sf::Text l = UI::makeText(lbl, gs.fontMono, 11, UI::TEXT2);
        l.setPosition(342, ry); w.draw(l);
        sf::Text v = UI::makeText(val, gs.fontOrb, 14, col);
        v.setPosition(492, ry); w.draw(v);
        ry += 24.f;
    };
    auto& p = gs.profile;
    statRow("GAMES", std::to_string(p.gamesPlayed), UI::TEXT);
    statRow("WINS",  std::to_string(p.wins),  UI::GREEN);
    statRow("LOSSES",std::to_string(p.losses), UI::RED);
    int winRate = p.gamesPlayed > 0 ? p.wins * 100 / p.gamesPlayed : 0;
    statRow("WIN RATE", std::to_string(winRate)+"%", UI::ACCENT);

    // Performance
    UI::drawPanel(w, 330, 284, 220, 180, sf::Color(UI::ACCENT3.r,UI::ACCENT3.g,UI::ACCENT3.b,30), UI::BG2, 8.f);
    sf::Text plbl = UI::makeText("PERFORMANCE", gs.fontOrb, 9, UI::TEXT2);
    plbl.setPosition(342, 292); w.draw(plbl);
    ry = 310.f;
    statRow("BEST WPM",  std::to_string(p.bestWpm), UI::ACCENT);
    statRow("BEST SCORE",std::to_string(p.bestScore), UI::ACCENT3);
    statRow("AVG ACC",   std::to_string(p.avgAcc)+"%", UI::GREEN);
    statRow("ACTIVE",    av.symbol+" "+av.name.substr(0,6), av.accentColor);

    // Back
    sf::Vector2i m = sf::Mouse::getPosition(w);
    bool hov = (m.x >= 300 && m.x <= 500 && m.y >= 530 && m.y <= 570);
    UI::drawPanel(w, 300, 530, 200, 40, hov ? UI::ACCENT2 : UI::TEXT2,
                  hov ? sf::Color(255,60,110,20) : UI::BG2, 8.f);
    sf::Text back = UI::makeText("BACK TO HQ", gs.fontOrb, 13, hov ? UI::ACCENT2 : UI::TEXT2);
    UI::centerText(back, 400.f, 550.f);
    w.draw(back);

    if (gs.settings.scanlines) UI::drawScanlines(w);
}
