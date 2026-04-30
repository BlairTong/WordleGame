#pragma once

#include "Tile.hpp"
#include <array>
#include <string>
#include <vector>

class Board {
public:
    static constexpr int Rows = 6;
    static constexpr int Cols = 5;

    Board();

    void setPosition(float x, float y);
    void update(float dt);
    void draw(sf::RenderWindow& window, const sf::Font& font) const;

    bool addLetter(char c);
    bool removeLetter();
    std::string getCurrentWord() const;
    bool isRowFull() const;
    bool isRowAnimating() const;
    bool isRowShaking() const;

    void applyResult(const std::string& target);
    void triggerShake();
    bool isGameOver() const { return gameOver; }
    bool hasWon() const { return won; }
    int getCurrentRow() const { return currentRow; }

    std::array<Tile::State, Cols> getRowResult(const std::string& target, const std::string& guess) const;

private:
    std::array<std::array<Tile, Cols>, Rows> tiles;
    int currentRow = 0;
    int currentCol = 0;
    int lastAnimatedRow = -1;
    float baseX = 0;
    float baseY = 0;
    bool gameOver = false;
    bool won = false;
};
