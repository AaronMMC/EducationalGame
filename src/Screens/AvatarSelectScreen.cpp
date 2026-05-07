#include "AvatarSelectScreen.h"
#include "../UIHelpers.h"

static const float CARD_W = 170.f, CARD_H = 200.f;
static const float CARD_START_X = 40.f, CARD_Y = 180.f, CARD_GAP = 14.f;

sf::FloatRect AvatarSelectScreen::cardBounds(int i) const {
    return {CARD_START_X + i * (CARD_W + CARD_GAP), CARD_Y, CARD_W, CARD_H};
}

void AvatarSelectScreen::onEnter(GameState& gs) {
    hoveredCard = -1;
}

void AvatarSelectScreen::handleEvent(sf::Event& event, GameState& gs) {
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2i m(event.mouseMove.x, event.mouseMove.y);
        hoveredCard = -1;
        for (int i = 0; i < 4; ++i) {
            auto b = cardBounds(i);
            if (m.x >= b.left && m.x <= b.left + b.width &&
                m.y >= b.top  && m.y <= b.top  + b.height)
                hoveredCard = i;
        }
    }
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i m(event.mouseButton.x, event.mouseButton.y);
        for (int i = 0; i < 4; ++i) {
            auto b = cardBounds(i);
            if (m.x >= b.left && m.x <= b.left + b.width &&
                m.y >= b.top  && m.y <= b.top  + b.height) {
                gs.profile.avatarId = i;
            }
        }
        // Confirm button
        if (m.x >= 490 && m.x <= 690 && m.y >= 420 && m.y <= 465) {
            if (gs.mode == GameMode::VS)
                gs.currentScreen = Screen::VS;
            else
                gs.currentScreen = Screen::Game;
        }
        // Back
        if (m.x >= 110 && m.x <= 310 && m.y >= 420 && m.y <= 465)
            gs.currentScreen = Screen::Title;
    }
    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::Escape)
        gs.currentScreen = Screen::Title;
}

void AvatarSelectScreen::update(float dt, GameState& gs) {}

void AvatarSelectScreen::draw(sf::RenderWindow& w, GameState& gs) {
    w.clear(UI::BG);
    UI::drawCornerBrackets(w, 10, 10, 780, 580, UI::ACCENT, 20.f);

    sf::Text title = UI::makeText("SELECT UNIT", gs.fontOrb, 22, UI::ACCENT);
    UI::centerText(title, 400.f, 60.f);
    UI::drawGlowText(w, title, UI::ACCENT, 4.f);

    std::string sub = (gs.mode == GameMode::VS)
        ? "CHOOSE YOUR COMBAT PROTOCOL - VS MODE"
        : "CHOOSE YOUR ENDLESS SIEGE UNIT";
    sf::Text subtitle = UI::makeText(sub, gs.fontMono, 11, UI::TEXT2);
    UI::centerText(subtitle, 400.f, 94.f);
    w.draw(subtitle);

    auto avatars = Avatar::all();
    for (int i = 0; i < 4; ++i)
        drawCard(w, gs, avatars[i], i,
                 gs.profile.avatarId == i, hoveredCard == i);

    // Back
    bool hb = false;
    sf::Vector2i m = sf::Mouse::getPosition(w);
    if (m.x >= 110 && m.x <= 310 && m.y >= 420 && m.y <= 465) hb = true;
    UI::drawPanel(w, 110, 420, 200, 45, hb ? UI::ACCENT2 : UI::TEXT2,
                  hb ? sf::Color(255,60,110,20) : UI::BG2, 8.f);
    sf::Text back = UI::makeText("BACK", gs.fontOrb, 13, hb ? UI::ACCENT2 : UI::TEXT2);
    UI::centerText(back, 210.f, 442.f);
    w.draw(back);

    // Confirm
    bool selected = (gs.profile.avatarId >= 0);
    bool hc = (m.x >= 490 && m.x <= 690 && m.y >= 420 && m.y <= 465);
    sf::Color confBorder = selected ? (hc ? UI::ACCENT : sf::Color(UI::ACCENT.r,UI::ACCENT.g,UI::ACCENT.b,180)) : UI::TEXT3;
    UI::drawPanel(w, 490, 420, 200, 45, confBorder,
                  (hc && selected) ? sf::Color(0,240,255,20) : UI::BG2, 8.f);
    sf::Text conf = UI::makeText("CONFIRM", gs.fontOrb, 13, selected ? confBorder : UI::TEXT3);
    UI::centerText(conf, 590.f, 442.f);
    w.draw(conf);

    if (gs.settings.scanlines) UI::drawScanlines(w);
}

void AvatarSelectScreen::drawCard(sf::RenderWindow& w, GameState& gs,
                                   const Avatar& av, int i,
                                   bool selected, bool hovered) {
    auto b = cardBounds(i);
    sf::Color border = selected ? UI::ACCENT3 : (hovered ? UI::ACCENT : UI::TEXT3);
    sf::Color fill   = selected ? sf::Color(240,192,0,12) : UI::BG2;
    UI::drawPanel(w, b.left, b.top, b.width, b.height, border, fill, 8.f);

    // Symbol
    sf::Text sym = UI::makeText(av.symbol, gs.fontMono, 20, av.accentColor);
    UI::centerText(sym, b.left + b.width/2.f, b.top + 36.f);
    if (hovered || selected) UI::drawGlowText(w, sym, av.accentColor, 3.f);
    else w.draw(sym);

    // Name
    sf::Text name = UI::makeText(av.name, gs.fontOrb, 12, av.accentColor);
    name.setStyle(sf::Text::Bold);
    UI::centerText(name, b.left + b.width/2.f, b.top + 70.f);
    w.draw(name);

    // Description — wrap manually at ~22 chars
    std::string desc = av.description;
    float ty = b.top + 96.f;
    std::string word, line;
    auto flush = [&]() {
        sf::Text dt = UI::makeText(line, gs.fontMono, 9, UI::TEXT2);
        UI::centerText(dt, b.left + b.width/2.f, ty);
        w.draw(dt);
        ty += 15.f; line = "";
    };
    for (char c : desc + " ") {
        if (c == ' ') {
            if (line.size() + word.size() > 22) flush();
            if (!line.empty()) line += ' ';
            line += word; word = "";
        } else word += c;
    }
    if (!line.empty()) flush();

    // Tag
    sf::Text tag = UI::makeText(av.tagline, gs.fontMono, 9, av.accentColor);
    UI::centerText(tag, b.left + b.width/2.f, b.top + b.height - 20.f);
    w.draw(tag);
}
