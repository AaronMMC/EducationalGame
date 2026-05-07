#include "UIHelpers.h"
#include <cmath>
#include <algorithm>

namespace UI {

void centerText(sf::Text& text, float x, float y) {
    sf::FloatRect b = text.getLocalBounds();
    text.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    text.setPosition(x, y);
}

void drawPanel(sf::RenderWindow& w, float x, float y,
               float width, float height,
               sf::Color borderColor, sf::Color fillColor, float cs) {
    // Fill
    sf::ConvexShape fill(8);
    fill.setPoint(0, {x + cs,         y});
    fill.setPoint(1, {x + width,      y});
    fill.setPoint(2, {x + width,      y + height - cs});
    fill.setPoint(3, {x + width - cs, y + height});
    fill.setPoint(4, {x,              y + height});
    fill.setPoint(5, {x,              y + cs});
    fill.setFillColor(fillColor);
    fill.setOutlineColor(borderColor);
    fill.setOutlineThickness(1.f);
    w.draw(fill);
}

void drawCornerBrackets(sf::RenderWindow& w, float x, float y,
                        float width, float height, sf::Color color, float size) {
    auto drawBracket = [&](float bx, float by, float dx, float dy) {
        sf::RectangleShape h({size, 2.f});
        h.setPosition(bx, by);
        h.setFillColor(color);
        sf::RectangleShape v({2.f, size});
        v.setPosition(bx, by);
        v.setFillColor(color);
        // Flip based on direction
        if (dx < 0) h.setPosition(bx - size + 2.f, by);
        if (dy < 0) v.setPosition(bx, by - size + 2.f);
        w.draw(h);
        w.draw(v);
    };
    drawBracket(x,         y,          1,  1);
    drawBracket(x + width, y,         -1,  1);
    drawBracket(x,         y + height, 1, -1);
    drawBracket(x + width, y + height,-1, -1);
}

void drawHPBar(sf::RenderWindow& w, float x, float y,
               float width, float height, float pct,
               const std::string& label, const sf::Font& font) {
    // Background track
    sf::RectangleShape track({width, height});
    track.setPosition(x, y);
    track.setFillColor(BG3);
    track.setOutlineColor(TEXT3);
    track.setOutlineThickness(1.f);
    w.draw(track);

    // Fill — color shifts with HP
    sf::Color fillColor = pct > 0.6f ? GREEN : pct > 0.3f ? ORANGE : RED;
    if (pct > 0.f) {
        sf::RectangleShape fill({std::max(0.f, width * pct), height});
        fill.setPosition(x, y);
        fill.setFillColor(fillColor);
        w.draw(fill);
    }

    // Label
    if (!label.empty()) {
        sf::Text t;
        t.setFont(font);
        t.setCharacterSize(11);
        t.setFillColor(TEXT2);
        t.setString(label);
        t.setPosition(x, y - 16.f);
        w.draw(t);
    }
}

void drawProgressBar(sf::RenderWindow& w, float x, float y,
                     float width, float height, float pct,
                     sf::Color fill, sf::Color bg) {
    sf::RectangleShape track({width, height});
    track.setPosition(x, y);
    track.setFillColor(bg);
    w.draw(track);
    if (pct > 0.f) {
        sf::RectangleShape bar({width * std::clamp(pct, 0.f, 1.f), height});
        bar.setPosition(x, y);
        bar.setFillColor(fill);
        w.draw(bar);
    }
}

void drawScanlines(sf::RenderWindow& w) {
    auto size = w.getSize();
    sf::RectangleShape line({(float)size.x, 1.f});
    line.setFillColor(sf::Color(0, 240, 255, 6));
    for (unsigned y = 0; y < size.y; y += 4) {
        line.setPosition(0.f, (float)y);
        w.draw(line);
    }
}

sf::Text makeText(const std::string& str, const sf::Font& font,
                  unsigned size, sf::Color color) {
    sf::Text t;
    t.setFont(font);
    t.setCharacterSize(size);
    t.setFillColor(color);
    t.setString(str);
    return t;
}

void drawGlowText(sf::RenderWindow& w, sf::Text& text,
                  sf::Color glowColor, float spread) {
    sf::Text glow = text;
    glow.setFillColor(sf::Color(glowColor.r, glowColor.g, glowColor.b, 60));
    for (float dx = -spread; dx <= spread; dx += spread) {
        for (float dy = -spread; dy <= spread; dy += spread) {
            if (dx == 0 && dy == 0) continue;
            glow.setPosition(text.getPosition() + sf::Vector2f(dx, dy));
            w.draw(glow);
        }
    }
    w.draw(text);
}

void drawMultiplierBadge(sf::RenderWindow& w, float x, float y,
                         float value, bool active, bool danger,
                         const sf::Font& font) {
    std::string str = "x" + std::to_string((int)value);
    if (value != (int)value) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "x%.1f", value);
        str = buf;
    }
    sf::Color col = !active ? TEXT3 : danger ? ORANGE : ACCENT3;
    sf::Text t = makeText(str, font, 20, col);
    t.setPosition(x, y);
    if (active) drawGlowText(w, t, col, 3.f);
    else w.draw(t);
}

} // namespace UI
