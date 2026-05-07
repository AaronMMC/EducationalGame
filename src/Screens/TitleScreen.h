#pragma once
#include "IScreen.h"

class TitleScreen : public IScreen {
public:
    void onEnter(GameState& gs) override;
    void handleEvent(sf::Event& event, GameState& gs) override;
    void update(float dt, GameState& gs) override;
    void draw(sf::RenderWindow& w, GameState& gs) override;

private:
    float animTime = 0.f;
    int   hoveredBtn = -1;
};
