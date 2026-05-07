#pragma once
#include "IScreen.h"

class AvatarSelectScreen : public IScreen {
public:
    void onEnter(GameState& gs) override;
    void handleEvent(sf::Event& event, GameState& gs) override;
    void update(float dt, GameState& gs) override;
    void draw(sf::RenderWindow& window, GameState& gs) override;
private:
    int  hoveredCard = -1;
    void drawCard(sf::RenderWindow& w, GameState& gs,
                  const Avatar& av, int i, bool selected, bool hovered);
    sf::FloatRect cardBounds(int i) const;
};
