#pragma once
#include "IScreen.h"

class ResultScreen : public IScreen {
public:
    void onEnter(GameState& gs) override;
    void handleEvent(sf::Event& event, GameState& gs) override;
    void update(float dt, GameState& gs) override;
    void draw(sf::RenderWindow& window, GameState& gs) override;
private:
    float enterAnim = 0.f;
};
