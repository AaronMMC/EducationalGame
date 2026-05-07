#pragma once
#include "IScreen.h"

class ProfileScreen : public IScreen {
public:
    void onEnter(GameState& gs) override;
    void handleEvent(sf::Event& event, GameState& gs) override;
    void update(float dt, GameState& gs) override;
    void draw(sf::RenderWindow& window, GameState& gs) override;
private:
    bool nameFocused = false;
    std::string nameBuffer;
};
