#pragma once
#include "IScreen.h"

class VSScreen : public IScreen {
public:
    void onEnter(GameState& gs) override;
    void handleEvent(sf::Event& event, GameState& gs) override;
    void update(float dt, GameState& gs) override;
    void draw(sf::RenderWindow& w, GameState& gs) override;

private:
    float animTime   = 0.f;
    float phase      = 0.f;   // 0=connecting, 1=both lit, 2=countdown
    float countTimer = 0.f;
    int   countdown  = 3;
    bool  p1Ready    = false;
    bool  p2Ready    = false;
    float p1ConnectT = 0.f;
    float p2ConnectT = 0.f;
};
