#pragma once
#include "IScreen.h"
#include <string>

class ProfileScreen : public IScreen {
public:
    void onEnter(GameState& gs) override;
    void handleEvent(sf::Event& event, GameState& gs) override;
    void update(float dt, GameState& gs) override;
    void draw(sf::RenderWindow& w, GameState& gs) override;

private:
    std::string nameBuffer;
    bool        nameFocused = false;
};
