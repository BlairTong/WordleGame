#include "Game.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <vector>

Game::Game() : window(sf::VideoMode({WindowWidth, WindowHeight}), "Wordle") {
    window.setFramerateLimit(60);
}

bool Game::loadFont() {
    const std::vector<std::string> fontPaths = {
        "assets/fonts/DejaVuSans.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf"
    };

    for (const auto& path : fontPaths) {
        if (font.openFromFile(path)) {
            return true;
        }
    }
    std::cerr << "No suitable font found. Please place a .ttf font in assets/fonts/DejaVuSans.ttf" << std::endl;
    return false;
}

bool Game::initialize() {
    if (!loadFont()) return false;

    if (!dictionary.loadFromFile("assets/data/words.txt")) {
        std::cerr << "Failed to load dictionary." << std::endl;
        return false;
    }

    targetWord = dictionary.getRandomWord();
    if (targetWord.empty()) {
        std::cerr << "Dictionary is empty." << std::endl;
        return false;
    }

    float boardWidth = Board::Cols * (Tile::Size + Tile::Margin) - Tile::Margin;
    float boardHeight = Board::Rows * (Tile::Size + Tile::Margin) - Tile::Margin;
    board.setPosition((WindowWidth - boardWidth) / 2.f, 80.f);

    // Initialize keyboard mapping
    const std::string rows[] = {
        "QWERTYUIOP",
        "ASDFGHJKL",
        "ZXCVBNM"
    };
    for (const auto& row : rows) {
        for (char c : row) {
            keyboardColors[c] = {Tile::State::Empty, sf::Keyboard::Key::Unknown};
        }
    }

    initialized = true;
    return true;
}

void Game::run() {
    if (!initialized) return;

    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents() {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
        if (board.isGameOver() || waitingForAnimation) continue;

        if (const auto* te = event->getIf<sf::Event::TextEntered>()) {
            if (te->unicode >= 'a' && te->unicode <= 'z') {
                handleTextEntered(static_cast<char>(te->unicode));
            } else if (te->unicode >= 'A' && te->unicode <= 'Z') {
                handleTextEntered(static_cast<char>(te->unicode + 32));
            }
        }
        if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
            if (kp->code == sf::Keyboard::Key::Backspace) {
                handleBackspace();
            } else if (kp->code == sf::Keyboard::Key::Enter) {
                handleEnter();
            }
        }
    }
}

void Game::handleTextEntered(char c) {
    board.addLetter(c);
}

void Game::handleBackspace() {
    board.removeLetter();
}

void Game::handleEnter() {
    if (!board.isRowFull()) return;

    std::string guess = board.getCurrentWord();
    if (!dictionary.isValidWord(guess)) {
        board.triggerShake();
        shakeMessageTimer = 1.2f;
        return;
    }

    board.applyResult(targetWord);
    updateKeyboardColors(guess, targetWord);
    waitingForAnimation = true;
}

void Game::updateKeyboardColors(const std::string& guess, const std::string& target) {
    for (size_t i = 0; i < guess.size(); ++i) {
        char c = static_cast<char>(std::toupper(guess[i]));
        auto it = keyboardColors.find(c);
        if (it == keyboardColors.end()) continue;

        Tile::State newState = Tile::State::Wrong;
        if (guess[i] == target[i]) {
            newState = Tile::State::Correct;
        } else if (target.find(guess[i]) != std::string::npos) {
            newState = Tile::State::Present;
        }

        // Upgrade state priority: Correct > Present > Wrong > Empty
        auto current = it->second.state;
        if (newState == Tile::State::Correct) {
            it->second.state = newState;
        } else if (newState == Tile::State::Present && current != Tile::State::Correct) {
            it->second.state = newState;
        } else if (newState == Tile::State::Wrong && current == Tile::State::Empty) {
            it->second.state = newState;
        }
    }
}

void Game::update(float dt) {
    board.update(dt);

    if (waitingForAnimation && !board.isRowAnimating()) {
        waitingForAnimation = false;
    }

    if (shakeMessageTimer > 0.f) {
        shakeMessageTimer -= dt;
    }
}

void Game::render() {
    window.clear(sf::Color(0xf5, 0xf5, 0xf0));
    board.draw(window, font);
    drawKeyboard(window);

    if (shakeMessageTimer > 0.f) {
        drawShakeMessage(window);
    }

    if (board.isGameOver()) {
        drawOverlay(window);
    }

    window.display();
}

void Game::drawKeyboard(sf::RenderWindow& window) const {
    const std::vector<std::string> keyRows = {
        "QWERTYUIOP",
        "ASDFGHJKL",
        "ZXCVBNM"
    };

    float keyWidth = 42.f;
    float keyHeight = 54.f;
    float keyMargin = 5.f;
    float startY = 560.f;

    for (size_t r = 0; r < keyRows.size(); ++r) {
        const auto& row = keyRows[r];
        float rowWidth = row.size() * (keyWidth + keyMargin) - keyMargin;
        float offsetX = (WindowWidth - rowWidth) / 2.f + r * 10.f;

        for (size_t i = 0; i < row.size(); ++i) {
            char c = row[i];
            sf::RectangleShape key({keyWidth, keyHeight});
            key.setPosition({offsetX + i * (keyWidth + keyMargin), startY + r * (keyHeight + keyMargin)});

            auto it = keyboardColors.find(c);
            if (it != keyboardColors.end() && it->second.state != Tile::State::Empty) {
                if (it->second.state == Tile::State::Correct)
                    key.setFillColor(sf::Color(0x53, 0x8d, 0x4e));
                else if (it->second.state == Tile::State::Present)
                    key.setFillColor(sf::Color(0xb5, 0x9f, 0x3b));
                else
                    key.setFillColor(sf::Color(0x3a, 0x3a, 0x3c));
            } else {
                key.setFillColor(sf::Color(0xd3, 0xd6, 0xda));
            }

            key.setOutlineThickness(0);
            window.draw(key);

            sf::Text text(font, std::string(1, c), 20);
            text.setFillColor(sf::Color(0x1a, 0x1a, 0x1a));
            text.setStyle(sf::Text::Bold);
            sf::FloatRect bounds = text.getLocalBounds();
            text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
            text.setPosition({key.getPosition().x + keyWidth / 2.f, key.getPosition().y + keyHeight / 2.f});
            window.draw(text);
        }
    }
}

void Game::drawOverlay(sf::RenderWindow& window) const {
    sf::RectangleShape overlay({static_cast<float>(WindowWidth), static_cast<float>(WindowHeight)});
    overlay.setFillColor(sf::Color(0xf0, 0xf0, 0xf0, 220));
    window.draw(overlay);

    std::string message = board.hasWon() ? "You Win!" : "Game Over";
    sf::Text msgText(font, message, 48);
    msgText.setFillColor(sf::Color(0x1a, 0x1a, 0x1a));
    msgText.setStyle(sf::Text::Bold);
    sf::FloatRect mb = msgText.getLocalBounds();
    msgText.setOrigin({mb.position.x + mb.size.x / 2.f, mb.position.y + mb.size.y / 2.f});
    msgText.setPosition({WindowWidth / 2.f, WindowHeight / 2.f - 40.f});
    window.draw(msgText);

    sf::Text wordText(font, "Word: " + targetWord, 32);
    wordText.setFillColor(sf::Color(0x1a, 0x1a, 0x1a));
    wordText.setStyle(sf::Text::Bold);
    sf::FloatRect wb = wordText.getLocalBounds();
    wordText.setOrigin({wb.position.x + wb.size.x / 2.f, wb.position.y + wb.size.y / 2.f});
    wordText.setPosition({WindowWidth / 2.f, WindowHeight / 2.f + 30.f});
    window.draw(wordText);
}

void Game::drawShakeMessage(sf::RenderWindow& window) const {
    sf::Text msg(font, "Not in word list", 20);
    msg.setFillColor(sf::Color(0x1a, 0x1a, 0x1a));
    sf::FloatRect b = msg.getLocalBounds();
    msg.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
    msg.setPosition({WindowWidth / 2.f, 50.f});
    window.draw(msg);
}
