#pragma once
#include "IScreen.h"
#include <vector>

class GameScreen : public IScreen {
public:
    void onEnter(GameState& gs) override;
    void handleEvent(sf::Event& event, GameState& gs) override;
    void update(float dt, GameState& gs) override;
    void draw(sf::RenderWindow& window, GameState& gs) override;

private:
    float cursorPhase    = 0.f;
    float bossAlertTimer = 0.f;
    bool  showBossAlert  = false;
    int   bossCountdown  = 3;
    bool  phoenixNotice  = false;
    float phoenixTimer   = 0.f;

    void drawHUD(sf::RenderWindow& w, GameState& gs);
    void drawTypingArea(sf::RenderWindow& w, GameState& gs);
    void drawOpponentPanel(sf::RenderWindow& w, GameState& gs);
    void drawBossAlert(sf::RenderWindow& w, GameState& gs);
    void drawSentinelFreeze(sf::RenderWindow& w, GameState& gs);
    void drawPhoenixNotice(sf::RenderWindow& w, GameState& gs);

    void processChar(sf::Uint32 unicode, GameState& gs);
    void startRound(GameState& gs);
    void nextRound(GameState& gs);
    void endMatch(GameState& gs);
    void checkRoundComplete(GameState& gs);

    // Keyword syntax highlighting colors
    sf::Color getCharBaseColor(const std::string& text, size_t i) const;
};
