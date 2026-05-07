#pragma once
#include "IScreen.h"

class SettingsScreen : public IScreen {
public:
    void onEnter(GameState& gs) override;
    void handleEvent(sf::Event& event, GameState& gs) override;
    void update(float dt, GameState& gs) override;
    void draw(sf::RenderWindow& w, GameState& gs) override;

private:
    int dragSlider = -1; // 0=music, 1=sfx
};
