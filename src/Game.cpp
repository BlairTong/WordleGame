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

void Game::initButtons() {
    float totalWidth = 2 * ButtonWidth + 20.f; // two buttons with 20px gap
    float startX = (WindowWidth - totalWidth) / 2.f;
    float buttonY = 748.f;

    restartButton.bounds = {{startX, buttonY}, {ButtonWidth, ButtonHeight}};
    restartButton.label = "Restart";

    giveUpButton.bounds = {{startX + ButtonWidth + 20.f, buttonY}, {ButtonWidth, ButtonHeight}};
    giveUpButton.label = "Give Up";
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

    initButtons();
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
            continue;
        }

        // Mouse clicks — always processed
        if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (pendingConfirm != ConfirmAction::None) {
                handleConfirmClick(mb->position.x, mb->position.y);
            } else {
                handleMouseClick(mb->position.x, mb->position.y);
            }
            continue;
        }

        // Escape cancels confirm dialog
        if (pendingConfirm != ConfirmAction::None) {
            if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Escape) {
                    pendingConfirm = ConfirmAction::None;
                    continue;
                }
            }
        }

        // Block game input when confirm dialog is showing
        if (pendingConfirm != ConfirmAction::None) continue;

        // Block game input when game over or animating
        if (board.isGameOver() || waitingForAnimation) continue;

        if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
            if (kp->code == sf::Keyboard::Key::Backspace) {
                handleBackspace();
            } else if (kp->code == sf::Keyboard::Key::Enter) {
                handleEnter();
            }
            continue;
        }

        if (const auto* te = event->getIf<sf::Event::TextEntered>()) {
            if (te->unicode >= 'a' && te->unicode <= 'z') {
                handleTextEntered(static_cast<char>(te->unicode));
            } else if (te->unicode >= 'A' && te->unicode <= 'Z') {
                handleTextEntered(static_cast<char>(te->unicode + 32));
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
    drawButtons(window);

    if (shakeMessageTimer > 0.f) {
        drawShakeMessage(window);
    }

    if (board.isGameOver()) {
        drawOverlay(window);
    }

    if (pendingConfirm != ConfirmAction::None) {
        drawConfirmDialog(window);
    }

    window.display();
}

void Game::handleMouseClick(float x, float y) {
    sf::Vector2f pos(x, y);

    // Overlay restart button (game over state)
    if (board.isGameOver()) {
        float btnW = 160.f;
        float btnH = 48.f;
        float btnX = WindowWidth / 2.f - btnW / 2.f;
        float btnY = WindowHeight / 2.f + 50.f;
        if (x >= btnX && x <= btnX + btnW && y >= btnY && y <= btnY + btnH) {
            pendingConfirm = ConfirmAction::Restart;
            return;
        }
        return;
    }

    if (restartButton.bounds.contains(pos)) {
        pendingConfirm = ConfirmAction::Restart;
    } else if (giveUpButton.bounds.contains(pos)) {
        pendingConfirm = ConfirmAction::GiveUp;
    }
}

void Game::handleConfirmClick(float x, float y) {
    // Confirm dialog box centered at (300, 400) with size 300x150
    float boxX = WindowWidth / 2.f - 150.f;
    float boxY = WindowHeight / 2.f - 75.f;
    float boxW = 300.f;
    float boxH = 150.f;

    if (x < boxX || x > boxX + boxW || y < boxY || y > boxY + boxH) {
        return;
    }

    // Yes button: left half of the bottom area
    float btnY = boxY + boxH - 48.f;
    float btnW = 100.f;
    float btnH = 32.f;
    float yesX = boxX + 40.f;
    float cancelX = boxX + boxW - 40.f - btnW;

    if (x >= yesX && x <= yesX + btnW && y >= btnY && y <= btnY + btnH) {
        if (pendingConfirm == ConfirmAction::Restart) {
            restart();
        } else if (pendingConfirm == ConfirmAction::GiveUp) {
            forfeit();
        }
        pendingConfirm = ConfirmAction::None;
    } else if (x >= cancelX && x <= cancelX + btnW && y >= btnY && y <= btnY + btnH) {
        pendingConfirm = ConfirmAction::None;
    }
}

void Game::drawButtons(sf::RenderWindow& window) const {
    auto drawButton = [&](const Button& btn, bool enabled, bool highlighted) {
        sf::RectangleShape rect({btn.bounds.size.x, btn.bounds.size.y});
        rect.setPosition(btn.bounds.position);

        if (highlighted) {
            rect.setFillColor(sf::Color(0x53, 0x8d, 0x4e));
            rect.setOutlineThickness(0);
        } else if (enabled) {
            rect.setFillColor(sf::Color(0xd3, 0xd6, 0xda));
            rect.setOutlineThickness(2);
            rect.setOutlineColor(sf::Color(0x87, 0x8a, 0x8c));
        } else {
            rect.setFillColor(sf::Color(0xa0, 0xa0, 0xa0));
            rect.setOutlineThickness(2);
            rect.setOutlineColor(sf::Color(0x87, 0x8a, 0x8c));
        }

        window.draw(rect);

        sf::Text text(font, btn.label, 16);
        text.setFillColor(highlighted ? sf::Color::White : sf::Color(0x1a, 0x1a, 0x1a));
        text.setStyle(highlighted ? sf::Text::Bold : sf::Text::Regular);
        sf::FloatRect tb = text.getLocalBounds();
        text.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
        text.setPosition({
            btn.bounds.position.x + btn.bounds.size.x / 2.f,
            btn.bounds.position.y + btn.bounds.size.y / 2.f
        });
        window.draw(text);
    };

    bool gameOver = board.isGameOver();
    if (!gameOver) {
        drawButton(restartButton, true, false);
        drawButton(giveUpButton, true, false);
    }
}

void Game::drawConfirmDialog(sf::RenderWindow& window) const {
    // Dimming overlay
    sf::RectangleShape overlay({static_cast<float>(WindowWidth), static_cast<float>(WindowHeight)});
    overlay.setFillColor(sf::Color(0x00, 0x00, 0x00, 120));
    window.draw(overlay);

    // Dialog box
    float boxW = 300.f;
    float boxH = 150.f;
    float boxX = WindowWidth / 2.f - boxW / 2.f;
    float boxY = WindowHeight / 2.f - boxH / 2.f;

    sf::RectangleShape box({boxW, boxH});
    box.setPosition({boxX, boxY});
    box.setFillColor(sf::Color(0xfa, 0xfa, 0xfa));
    box.setOutlineThickness(2);
    box.setOutlineColor(sf::Color(0x87, 0x8a, 0x8c));
    window.draw(box);

    // Title text
    std::string title = (pendingConfirm == ConfirmAction::Restart) ? "Restart game?" : "Give up?";
    sf::Text titleText(font, title, 22);
    titleText.setFillColor(sf::Color(0x1a, 0x1a, 0x1a));
    sf::FloatRect tb = titleText.getLocalBounds();
    titleText.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
    titleText.setPosition({WindowWidth / 2.f, boxY + 40.f});
    window.draw(titleText);

    // Yes button
    float btnW = 100.f;
    float btnH = 32.f;
    float btnY = boxY + boxH - 48.f;
    float yesX = boxX + 40.f;
    float cancelX = boxX + boxW - 40.f - btnW;

    sf::RectangleShape yesBtn({btnW, btnH});
    yesBtn.setPosition({yesX, btnY});
    yesBtn.setFillColor(sf::Color(0xd3, 0xd6, 0xda));
    yesBtn.setOutlineThickness(1);
    yesBtn.setOutlineColor(sf::Color(0x87, 0x8a, 0x8c));
    window.draw(yesBtn);

    sf::Text yesText(font, "Yes", 16);
    yesText.setFillColor(sf::Color(0x1a, 0x1a, 0x1a));
    sf::FloatRect yb = yesText.getLocalBounds();
    yesText.setOrigin({yb.position.x + yb.size.x / 2.f, yb.position.y + yb.size.y / 2.f});
    yesText.setPosition({yesX + btnW / 2.f, btnY + btnH / 2.f});
    window.draw(yesText);

    // Cancel button
    sf::RectangleShape cancelBtn({btnW, btnH});
    cancelBtn.setPosition({cancelX, btnY});
    cancelBtn.setFillColor(sf::Color(0xd3, 0xd6, 0xda));
    cancelBtn.setOutlineThickness(1);
    cancelBtn.setOutlineColor(sf::Color(0x87, 0x8a, 0x8c));
    window.draw(cancelBtn);

    sf::Text cancelText(font, "Cancel", 16);
    cancelText.setFillColor(sf::Color(0x1a, 0x1a, 0x1a));
    sf::FloatRect cb = cancelText.getLocalBounds();
    cancelText.setOrigin({cb.position.x + cb.size.x / 2.f, cb.position.y + cb.size.y / 2.f});
    cancelText.setPosition({cancelX + btnW / 2.f, btnY + btnH / 2.f});
    window.draw(cancelText);
}

void Game::restart() {
    board.reset();
    targetWord = dictionary.getRandomWord();
    waitingForAnimation = false;
    shakeMessageTimer = 0.f;
    playerForfeited = false;

    for (auto& [c, ks] : keyboardColors) {
        ks.state = Tile::State::Empty;
    }
}

void Game::forfeit() {
    playerForfeited = true;
    board.forceGameOver();
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

    std::string message;
    if (board.hasWon()) {
        message = "You Win!";
    } else if (playerForfeited) {
        message = "You gave up!";
    } else {
        message = "Game Over";
    }

    sf::Text msgText(font, message, 48);
    msgText.setFillColor(sf::Color(0x1a, 0x1a, 0x1a));
    msgText.setStyle(sf::Text::Bold);
    sf::FloatRect mb = msgText.getLocalBounds();
    msgText.setOrigin({mb.position.x + mb.size.x / 2.f, mb.position.y + mb.size.y / 2.f});
    msgText.setPosition({WindowWidth / 2.f, WindowHeight / 2.f - 60.f});
    window.draw(msgText);

    sf::Text wordText(font, "Word: " + targetWord, 32);
    wordText.setFillColor(sf::Color(0x1a, 0x1a, 0x1a));
    wordText.setStyle(sf::Text::Bold);
    sf::FloatRect wb = wordText.getLocalBounds();
    wordText.setOrigin({wb.position.x + wb.size.x / 2.f, wb.position.y + wb.size.y / 2.f});
    wordText.setPosition({WindowWidth / 2.f, WindowHeight / 2.f});
    window.draw(wordText);

    // Prominent Restart button
    float btnW = 160.f;
    float btnH = 48.f;
    float btnX = WindowWidth / 2.f - btnW / 2.f;
    float btnY = WindowHeight / 2.f + 50.f;

    sf::RectangleShape restartBtn({btnW, btnH});
    restartBtn.setPosition({btnX, btnY});
    restartBtn.setFillColor(sf::Color(0x53, 0x8d, 0x4e));
    window.draw(restartBtn);

    sf::Text btnText(font, "Restart", 22);
    btnText.setFillColor(sf::Color::White);
    btnText.setStyle(sf::Text::Bold);
    sf::FloatRect tb = btnText.getLocalBounds();
    btnText.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
    btnText.setPosition({btnX + btnW / 2.f, btnY + btnH / 2.f});
    window.draw(btnText);
}

void Game::drawShakeMessage(sf::RenderWindow& window) const {
    sf::Text msg(font, "Not in word list", 20);
    msg.setFillColor(sf::Color(0x1a, 0x1a, 0x1a));
    sf::FloatRect b = msg.getLocalBounds();
    msg.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
    msg.setPosition({WindowWidth / 2.f, 50.f});
    window.draw(msg);
}
