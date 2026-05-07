#pragma once
#include <SFML/Graphics.hpp>
#include "../GameState.h"

class IScreen {
public:
    virtual ~IScreen() = default;
    virtual void onEnter(GameState& gs) {}
    virtual void handleEvent(sf::Event& event, GameState& gs) = 0;
    virtual void update(float dt, GameState& gs) = 0;
    virtual void draw(sf::RenderWindow& window, GameState& gs) = 0;
};
