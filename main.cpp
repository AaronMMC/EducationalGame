#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <cmath>

enum class State { Waiting, Playing, Finished };

class Stats {
public:
    int totalTyped = 0;
    int errors = 0;
    int correctChars = 0;
    float timeLimit = 30.0f;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> endTime;
    bool isFinished = false;

    void start() {
        startTime = std::chrono::steady_clock::now();
        isFinished = false;
        totalTyped = 0;
        errors = 0;
        correctChars = 0;
    }

    void finish() {
        endTime = std::chrono::steady_clock::now();
        isFinished = true;
    }

    float getElapsedSeconds() {
        auto end = isFinished ? endTime : std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = end - startTime;
        return elapsed.count();
    }

    float getRemainingTime() {
        return std::max(0.0f, timeLimit - getElapsedSeconds());
    }

    int getWPM() {
        float minutes = getElapsedSeconds() / 60.0f;
        if (minutes <= 0) return 0;
        return static_cast<int>((correctChars / 5.0f) / minutes);
    }

    int getAccuracy() {
        if (totalTyped == 0) return 100;
        return static_cast<int>((static_cast<float>(correctChars) / totalTyped) * 100.0f);
    }
};

class Game {
private:
    sf::RenderWindow window;
    sf::Font font;
    Stats stats;
    State currentState = State::Waiting;

    std::vector<std::string> snippets;
    std::string targetText;
    std::string currentInput;
    sf::RectangleShape cursor;
    std::vector<sf::Color> targetColors;

    void loadSnippets() {
        std::ifstream file("snippets.txt");
        if (!file.is_open()) {
            std::cout << "Could not find snippets.txt! Loading fallback." << std::endl;
            snippets.push_back("int main() {\n    return 0;\n}");
            return;
        }

        std::string currentSnippet = "";
        std::string line;
        while (std::getline(file, line)) {
            if (line == "---") {
                if (!currentSnippet.empty()) {
                    currentSnippet.pop_back();
                    snippets.push_back(currentSnippet);
                    currentSnippet = "";
                }
            } else {
                currentSnippet += line + "\n";
            }
        }
        if (!currentSnippet.empty()) {
            currentSnippet.pop_back();
            snippets.push_back(currentSnippet);
        }
    }

    void reset() {
        targetText = snippets[std::rand() % snippets.size()];
        currentInput = "";
        currentState = State::Waiting;
        cursor.setSize(sf::Vector2f(3.f, 32.f));
        cursor.setFillColor(sf::Color(220, 180, 50));
        stats.isFinished = false;

        targetColors.assign(targetText.length(), sf::Color(100, 100, 100));

        std::vector<std::pair<std::string, sf::Color>> keywords = {
            {"int ", sf::Color(198, 120, 221)},
            {"void ", sf::Color(198, 120, 221)},
            {"class ", sf::Color(198, 120, 221)},
            {"public:", sf::Color(198, 120, 221)},
            {"for ", sf::Color(198, 120, 221)},
            {"return ", sf::Color(198, 120, 221)},
            {"#include ", sf::Color(198, 120, 221)},
            {"std::cout", sf::Color(229, 192, 123)},
            {"std::vector", sf::Color(229, 192, 123)},
            {"std::string", sf::Color(229, 192, 123)}
        };

        for (const auto& kw : keywords) {
            size_t pos = targetText.find(kw.first);
            while (pos != std::string::npos) {
                for (size_t i = 0; i < kw.first.length(); ++i) {
                    targetColors[pos + i] = kw.second;
                }
                pos = targetText.find(kw.first, pos + kw.first.length());
            }
        }

        bool inString = false;
        for (size_t i = 0; i < targetText.length(); ++i) {
            if (targetText[i] == '"') inString = !inString;
            if (inString || targetText[i] == '"') targetColors[i] = sf::Color(152, 195, 121);
        }
    }

    void processChar(char c) {
        if (currentInput.length() < targetText.length()) {
            currentInput += c;
            stats.totalTyped++;

            if (currentInput.back() == targetText[currentInput.length() - 1]) {
                stats.correctChars++;
            } else {
                stats.errors++;
            }
        }
    }

    void handleInput(sf::Uint32 unicode) {
        if (currentState == State::Finished) return;
        if (unicode == 27) return;

        if (unicode == 13) unicode = 10;

        if (currentState == State::Waiting && unicode != '\b' && unicode != 9 && unicode != 10) {
            currentState = State::Playing;
            stats.start();
        }

        if (unicode == '\b' && !currentInput.empty()) {
            currentInput.pop_back();
        } else if (unicode == 9) {
            for (int i = 0; i < 4; i++) {
                processChar(' ');
            }
        } else if (unicode >= 32 || unicode == 10) {
            processChar(static_cast<char>(unicode));
        }

        if (currentInput.length() == targetText.length()) {
            currentState = State::Finished;
            stats.finish();
        }
    }

    void centerText(sf::Text& text, float x, float y) {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
        text.setPosition(x, y);
    }

    void drawTextAndCursor() {
        sf::Text charDisplay("", font, 32);
        float startX = 50.f;
        float currentX = startX;
        float currentY = 150.f;
        bool cursorDrawn = false;

        if (currentInput.empty() && currentState != State::Finished) {
            cursor.setPosition(currentX, currentY + 8.f);
            window.draw(cursor);
            cursorDrawn = true;
        }

        for (size_t i = 0; i < targetText.length(); ++i) {
            if (targetText[i] == '\n') {
                currentX = startX;
                currentY += 40.f;
                if (i == currentInput.length() - 1 && !cursorDrawn && currentState != State::Finished) {
                    cursor.setPosition(currentX, currentY + 8.f);
                    window.draw(cursor);
                    cursorDrawn = true;
                }
                continue;
            }

            charDisplay.setString(targetText[i]);
            charDisplay.setPosition(currentX, currentY);

            if (i < currentInput.length()) {
                if (currentInput[i] == targetText[i]) {
                    charDisplay.setFillColor(sf::Color(255, 255, 255));
                } else {
                    charDisplay.setFillColor(sf::Color(200, 70, 70));
                    if (targetText[i] == ' ') {
                        sf::RectangleShape errorSpace(sf::Vector2f(15.f, 5.f));
                        errorSpace.setFillColor(sf::Color(200, 70, 70, 150));
                        errorSpace.setPosition(currentX, currentY + 25.f);
                        window.draw(errorSpace);
                    }
                }
            } else {
                charDisplay.setFillColor(targetColors[i]);
            }

            window.draw(charDisplay);
            currentX += font.getGlyph(targetText[i], 32, false).advance;

            if (i == currentInput.length() - 1 && currentState != State::Finished && !cursorDrawn) {
                cursor.setPosition(currentX, currentY + 8.f);
                window.draw(cursor);
                cursorDrawn = true;
            }
        }
    }

    void drawUI() {
        sf::Text uiText("", font, 24);
        uiText.setFillColor(sf::Color(150, 150, 150));

        if (currentState == State::Playing) {
            int remaining = static_cast<int>(stats.getRemainingTime());
            uiText.setString(std::to_string(remaining) + "s  |  WPM: " + std::to_string(stats.getWPM()) + "  |  ACC: " + std::to_string(stats.getAccuracy()) + "%");
            centerText(uiText, 400.f, 50.f);
            window.draw(uiText);
        } else if (currentState == State::Waiting) {
            uiText.setString("Start typing to begin...");
            centerText(uiText, 400.f, 50.f);
            window.draw(uiText);
        } else if (currentState == State::Finished) {
            sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
            overlay.setFillColor(sf::Color(30, 30, 35, 220));
            window.draw(overlay);

            uiText.setString("Test Complete! Press TAB + ENTER to restart.");
            centerText(uiText, 400.f, 200.f);
            window.draw(uiText);

            sf::Text results("WPM: " + std::to_string(stats.getWPM()) + "\nAccuracy: " + std::to_string(stats.getAccuracy()) + "%\nErrors: " + std::to_string(stats.errors), font, 40);
            results.setFillColor(sf::Color(220, 180, 50));
            centerText(results, 400.f, 300.f);
            window.draw(results);
        }
    }

public:
    Game() : window(sf::VideoMode(800, 600), "SynType") {
        window.setFramerateLimit(60);
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cout << "Could not find Arial on C: drive!" << std::endl;
            exit(-1);
        }
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        loadSnippets();
        reset();
    }

    void run() {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
                if (event.type == sf::Event::TextEntered) {
                    sf::Uint32 unicode = event.text.unicode;

                    if ((unicode == 13 || unicode == 10) && sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)) {
                        reset();
                    } else {
                        handleInput(unicode);
                    }
                }
            }

            if (currentState == State::Playing && stats.getRemainingTime() <= 0) {
                currentState = State::Finished;
                stats.finish();
            }

            float time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0f;
            cursor.setFillColor(sf::Color(220, 180, 50, static_cast<sf::Uint8>(std::abs(std::sin(time * 5.0f)) * 255)));

            window.clear(sf::Color(30, 30, 35));
            drawTextAndCursor();
            drawUI();
            window.display();
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}