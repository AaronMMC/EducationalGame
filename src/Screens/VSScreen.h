#pragma once
#include "IScreen.h"

class VSScreen : public IScreen {
public:
    void onEnter(GameState& gs) override;
    void handleEvent(sf::Event& event, GameState& gs) override;
    void update(float dt, GameState& gs) override;
    void draw(sf::RenderWindow& window, GameState& gs) override;
private:
    float timer      = 0.f;
    float barFill    = 0.f;
    int   countdown  = 3;
    bool  p1Lit      = false;
    bool  p2Lit      = false;
    bool  started    = false;
    float vsPulse    = 0.f;
};
