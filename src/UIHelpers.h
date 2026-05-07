#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace UI {
    // Color palette
    const sf::Color BG        {10,  12,  16};
    const sf::Color BG2       {17,  19,  24};
    const sf::Color BG3       {26,  29,  36};
    const sf::Color ACCENT    {0,   240, 255};
    const sf::Color ACCENT2   {255, 60,  110};
    const sf::Color ACCENT3   {240, 192, 0};
    const sf::Color ACCENT4   {123, 47,  255};
    const sf::Color TEXT      {232, 234, 240};
    const sf::Color TEXT2     {122, 128, 144};
    const sf::Color TEXT3     {58,  63,  74};
    const sf::Color GREEN     {0,   230, 118};
    const sf::Color RED       {255, 23,  68};
    const sf::Color ORANGE    {255, 145, 0};

    void centerText(sf::Text& text, float x, float y);

    // Draw a clipped "arcade" panel border
    void drawPanel(sf::RenderWindow& w, float x, float y,
                   float width, float height,
                   sf::Color borderColor = ACCENT,
                   sf::Color fillColor   = BG2,
                   float clipSize        = 10.f);

    // Draw corner bracket decorations
    void drawCornerBrackets(sf::RenderWindow& w, float x, float y,
                            float width, float height,
                            sf::Color color = ACCENT,
                            float size      = 20.f);

    // Draw an HP bar
    void drawHPBar(sf::RenderWindow& w, float x, float y,
                   float width, float height,
                   float pct,          // 0..1
                   const std::string& label,
                   const sf::Font& font);

    // Draw a scanline overlay rect
    void drawScanlines(sf::RenderWindow& w);

    // Make a label sf::Text with common defaults
    sf::Text makeText(const std::string& str, const sf::Font& font,
                      unsigned size, sf::Color color);

    // Glow helper: draw text twice — once blurred (larger, transparent) then crisp
    void drawGlowText(sf::RenderWindow& w, sf::Text& text,
                      sf::Color glowColor, float spread = 4.f);

    // Draw a simple progress bar (no label)
    void drawProgressBar(sf::RenderWindow& w, float x, float y,
                         float width, float height,
                         float pct, sf::Color fill, sf::Color bg = BG3);

    // Multiplier badge
    void drawMultiplierBadge(sf::RenderWindow& w, float x, float y,
                             float value, bool active, bool danger,
                             const sf::Font& font);
}
