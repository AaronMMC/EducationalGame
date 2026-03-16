#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Stats.h"

enum class State { Waiting, Playing, Finished };

class Game {
private:
    sf::RenderWindow window;
    sf::Font font;
    Stats stats;
    State currentState;

    std::vector<std::string> snippets;
    std::string targetText;
    std::string currentInput;
    sf::RectangleShape cursor;
    std::vector<sf::Color> targetColors;

    void loadSnippets();
    void reset();
    void processChar(char c);
    void handleInput(sf::Uint32 unicode);
    void centerText(sf::Text& text, float x, float y);
    void drawTextAndCursor();
    void drawUI();

public:
    Game();
    void run();
};