#pragma once
#include "IScreen.h"

class SPSelectScreen : public IScreen {
public:
    void handleEvent(sf::Event& event, GameState& gs) override;
    void update(float dt, GameState& gs) override {}
    void draw(sf::RenderWindow& window, GameState& gs) override;
private:
    int hovered = -1;
};
