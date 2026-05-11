#pragma once

#include "Board.hpp"
#include "Dictionary.hpp"
#include <SFML/Graphics.hpp>
#include <map>
#include <string>

enum class ConfirmAction { None, Restart, GiveUp };

class Game {
public:
    Game();
    bool initialize();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();

    void handleTextEntered(char c);
    void handleBackspace();
    void handleEnter();
    void updateKeyboardColors(const std::string& guess, const std::string& target);
    void drawKeyboard(sf::RenderWindow& window) const;
    void drawOverlay(sf::RenderWindow& window) const;
    void drawShakeMessage(sf::RenderWindow& window) const;

    void initButtons();
    void handleMouseClick(float x, float y);
    void handleConfirmClick(float x, float y);
    void drawButtons(sf::RenderWindow& window) const;
    void drawConfirmDialog(sf::RenderWindow& window) const;
    void restart();
    void forfeit();

    bool loadFont();

    sf::RenderWindow window;
    sf::Font font;
    Board board;
    Dictionary dictionary;
    std::string targetWord;

    bool initialized = false;
    bool waitingForAnimation = false;
    float shakeMessageTimer = 0.f;
    bool playerForfeited = false;

    ConfirmAction pendingConfirm = ConfirmAction::None;

    struct Button {
        sf::FloatRect bounds;
        std::string label;
    };
    Button restartButton;
    Button giveUpButton;

    struct KeyState {
        Tile::State state = Tile::State::Empty;
        sf::Keyboard::Key key = sf::Keyboard::Key::Unknown;
    };
    std::map<char, KeyState> keyboardColors;

    static constexpr int WindowWidth = 600;
    static constexpr int WindowHeight = 800;
    static constexpr float ButtonWidth = 100.f;
    static constexpr float ButtonHeight = 36.f;
};
