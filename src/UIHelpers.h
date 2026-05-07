#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace UI {
    // Palette
    inline const sf::Color BG      = {8,   10,  14};
    inline const sf::Color BG2     = {14,  17,  24};
    inline const sf::Color BG3     = {20,  24,  34};
    inline const sf::Color ACCENT  = {0,   240, 255};   // cyan
    inline const sf::Color ACCENT2 = {255, 60,  110};   // magenta/red
    inline const sf::Color ACCENT3 = {240, 192, 0};     // gold
    inline const sf::Color GREEN   = {0,   230, 118};
    inline const sf::Color RED     = {255, 60,  60};
    inline const sf::Color ORANGE  = {255, 145, 0};
    inline const sf::Color TEXT    = {220, 225, 235};
    inline const sf::Color TEXT2   = {120, 130, 150};
    inline const sf::Color TEXT3   = {60,  70,  90};

    sf::Text makeText(const std::string& str, const sf::Font& font, unsigned size, sf::Color col);
    void centerText(sf::Text& t, float cx, float cy);
    void drawGlowText(sf::RenderWindow& w, sf::Text t, sf::Color glowCol, float radius);
    void drawHPBar(sf::RenderWindow& w, float x, float y, float width, float height,
                   float pct, const std::string& label, const sf::Font& font);
    void drawProgressBar(sf::RenderWindow& w, float x, float y, float width, float height,
                         float pct, sf::Color fill, sf::Color bg);
    void drawPanel(sf::RenderWindow& w, float x, float y, float width, float height,
                   sf::Color border, sf::Color bg, float cornerRadius = 0.f);
    void drawMultiplierBadge(sf::RenderWindow& w, float x, float y, float multi,
                              bool active, bool danger, const sf::Font& font);
    void drawScanlines(sf::RenderWindow& w);
    void drawCornerBrackets(sf::RenderWindow& w, float x, float y, float x2, float y2,
                             sf::Color col, float size);
}
