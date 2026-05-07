#include "UIHelpers.h"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace UI {

sf::Text makeText(const std::string& str, const sf::Font& font, unsigned size, sf::Color col) {
    sf::Text t;
    t.setFont(font);
    t.setString(str);
    t.setCharacterSize(size);
    t.setFillColor(col);
    return t;
}

void centerText(sf::Text& t, float cx, float cy) {
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    t.setPosition(cx, cy);
}

void drawGlowText(sf::RenderWindow& w, sf::Text t, sf::Color glowCol, float radius) {
    sf::Color gc = glowCol;
    for (int i = 3; i >= 1; --i) {
        gc.a = (sf::Uint8)(60 / i);
        t.setFillColor(gc);
        for (float dx = -radius; dx <= radius; dx += radius)
            for (float dy = -radius; dy <= radius; dy += radius) {
                if (dx == 0 && dy == 0) continue;
                sf::Vector2f orig = t.getPosition();
                t.move(dx, dy);
                w.draw(t);
                t.setPosition(orig);
            }
    }
    t.setFillColor(glowCol);
    w.draw(t);
}

void drawHPBar(sf::RenderWindow& w, float x, float y, float width, float height,
               float pct, const std::string& label, const sf::Font& font) {
    // Background
    sf::RectangleShape bg({width, height});
    bg.setPosition(x, y);
    bg.setFillColor({20, 25, 35});
    bg.setOutlineColor({40, 50, 70});
    bg.setOutlineThickness(1.f);
    w.draw(bg);

    // Fill color transitions red->orange->green
    sf::Color fill;
    if (pct > 0.6f)      fill = {0, 200, 100};
    else if (pct > 0.3f) fill = {255, 145, 0};
    else                 fill = {255, 50, 50};

    sf::RectangleShape bar({width * pct, height});
    bar.setPosition(x, y);
    bar.setFillColor(fill);
    w.draw(bar);

    // Shine strip
    sf::RectangleShape shine({width * pct, height * 0.3f});
    shine.setPosition(x, y);
    shine.setFillColor({255, 255, 255, 30});
    w.draw(shine);
}

void drawProgressBar(sf::RenderWindow& w, float x, float y, float width, float height,
                     float pct, sf::Color fill, sf::Color bg) {
    sf::RectangleShape bgR({width, height});
    bgR.setPosition(x, y);
    bgR.setFillColor(bg);
    w.draw(bgR);

    if (pct > 0.f) {
        sf::RectangleShape fillR({width * pct, height});
        fillR.setPosition(x, y);
        fillR.setFillColor(fill);
        w.draw(fillR);
    }
}

void drawPanel(sf::RenderWindow& w, float x, float y, float width, float height,
               sf::Color border, sf::Color bg, float /*cornerRadius*/) {
    sf::RectangleShape panel({width, height});
    panel.setPosition(x, y);
    panel.setFillColor(bg);
    panel.setOutlineColor(border);
    panel.setOutlineThickness(1.f);
    w.draw(panel);
}

void drawMultiplierBadge(sf::RenderWindow& w, float x, float y, float multi,
                          bool active, bool danger, const sf::Font& font) {
    sf::Color col = danger ? RED : (active ? ACCENT3 : TEXT3);

    sf::RectangleShape badge({56.f, 28.f});
    badge.setPosition(x, y);
    badge.setFillColor({(sf::Uint8)(col.r/6), (sf::Uint8)(col.g/6), (sf::Uint8)(col.b/6)});
    badge.setOutlineColor(col);
    badge.setOutlineThickness(1.f);
    w.draw(badge);

    std::ostringstream ss;
    ss << "x" << std::fixed << std::setprecision(1) << multi;
    sf::Text t = makeText(ss.str(), font, 14, col);
    centerText(t, x + 28.f, y + 14.f);
    w.draw(t);
}

void drawScanlines(sf::RenderWindow& w) {
    sf::RectangleShape line({800.f, 1.f});
    line.setFillColor({0, 0, 0, 28});
    for (float y = 0; y < 600.f; y += 3.f) {
        line.setPosition(0, y);
        w.draw(line);
    }
}

void drawCornerBrackets(sf::RenderWindow& w, float x, float y, float x2, float y2,
                         sf::Color col, float size) {
    auto drawL = [&](float ox, float oy, bool flipX, bool flipY) {
        float sx = flipX ? -1.f : 1.f;
        float sy = flipY ? -1.f : 1.f;

        sf::RectangleShape h({size, 2.f});
        h.setPosition(ox, oy);
        h.setSize({size * sx, 2.f});
        h.setFillColor(col);
        w.draw(h);

        sf::RectangleShape v({2.f, size});
        v.setPosition(ox, oy);
        v.setSize({2.f, size * sy});
        v.setFillColor(col);
        w.draw(v);
    };

    drawL(x,  y,  false, false);
    drawL(x2, y,  true,  false);
    drawL(x,  y2, false, true);
    drawL(x2, y2, true,  true);
}

} // namespace UI
