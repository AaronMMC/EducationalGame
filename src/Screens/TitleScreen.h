#pragma once
#include "IScreen.h"

class TitleScreen : public IScreen {
public:
    void onEnter(GameState& gs) override;
    void handleEvent(sf::Event& event, GameState& gs) override;
    void update(float dt, GameState& gs) override;
    void draw(sf::RenderWindow& window, GameState& gs) override;
private:
    float glowPhase = 0.f;
    float tickerX   = 900.f;
    int   hoveredBtn = -1;

    void drawLogo(sf::RenderWindow& w, GameState& gs);
    void drawMenu(sf::RenderWindow& w, GameState& gs);
    void drawTicker(sf::RenderWindow& w, GameState& gs);
    bool isMouseOverBtn(const sf::Vector2i& mouse, int btnIndex) const;
};
