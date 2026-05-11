#include "Board.hpp"

Board::Board() {
    for (int r = 0; r < Rows; ++r) {
        for (int c = 0; c < Cols; ++c) {
            tiles[r][c] = Tile();
        }
    }
}

void Board::setPosition(float x, float y) {
    baseX = x;
    baseY = y;
    for (int r = 0; r < Rows; ++r) {
        for (int c = 0; c < Cols; ++c) {
            tiles[r][c].setPosition(
                baseX + c * (Tile::Size + Tile::Margin),
                baseY + r * (Tile::Size + Tile::Margin)
            );
        }
    }
}

void Board::update(float dt) {
    for (int r = 0; r < Rows; ++r) {
        for (int c = 0; c < Cols; ++c) {
            tiles[r][c].update(dt);
        }
    }
}

void Board::draw(sf::RenderWindow& window, const sf::Font& font) const {
    for (int r = 0; r < Rows; ++r) {
        for (int c = 0; c < Cols; ++c) {
            tiles[r][c].draw(window, font);
        }
    }
}

bool Board::addLetter(char c) {
    if (gameOver || currentRow >= Rows || currentCol >= Cols) return false;
    tiles[currentRow][currentCol].setLetter(c);
    ++currentCol;
    return true;
}

bool Board::removeLetter() {
    if (gameOver || currentRow >= Rows || currentCol <= 0) return false;
    --currentCol;
    tiles[currentRow][currentCol].setLetter(' ');
    tiles[currentRow][currentCol].setState(Tile::State::Empty);
    return true;
}

std::string Board::getCurrentWord() const {
    std::string word;
    for (int c = 0; c < Cols; ++c) {
        word.push_back(tiles[currentRow][c].getLetter());
    }
    return word;
}

bool Board::isRowFull() const {
    return currentCol == Cols;
}

bool Board::isRowAnimating() const {
    if (lastAnimatedRow < 0 || lastAnimatedRow >= Rows) return false;
    for (int c = 0; c < Cols; ++c) {
        if (tiles[lastAnimatedRow][c].isAnimating()) return true;
    }
    return false;
}

bool Board::isRowShaking() const {
    if (currentRow < 0 || currentRow >= Rows) return false;
    for (int c = 0; c < Cols; ++c) {
        if (tiles[currentRow][c].isAnimating()) return true;
    }
    return false;
}

std::array<Tile::State, Board::Cols> Board::getRowResult(const std::string& target, const std::string& guess) const {
    std::array<Tile::State, Cols> result{};
    result.fill(Tile::State::Wrong);

    std::array<bool, Cols> targetUsed{};
    targetUsed.fill(false);

    // First pass: mark correct positions
    for (int i = 0; i < Cols; ++i) {
        if (guess[i] == target[i]) {
            result[i] = Tile::State::Correct;
            targetUsed[i] = true;
        }
    }

    // Second pass: mark present (yellow) for letters that exist elsewhere
    for (int i = 0; i < Cols; ++i) {
        if (result[i] == Tile::State::Correct) continue;
        for (int j = 0; j < Cols; ++j) {
            if (!targetUsed[j] && guess[i] == target[j]) {
                result[i] = Tile::State::Present;
                targetUsed[j] = true;
                break;
            }
        }
    }

    return result;
}

void Board::applyResult(const std::string& target) {
    std::string guess = getCurrentWord();
    auto result = getRowResult(target, guess);

    for (int c = 0; c < Cols; ++c) {
        tiles[currentRow][c].triggerFlip(result[c]);
    }

    if (guess == target) {
        won = true;
        gameOver = true;
    } else if (currentRow + 1 >= Rows) {
        gameOver = true;
    }

    lastAnimatedRow = currentRow;
    ++currentRow;
    currentCol = 0;
}

void Board::triggerShake() {
    for (int c = 0; c < Cols; ++c) {
        tiles[currentRow][c].triggerShake();
    }
}

void Board::reset() {
    for (int r = 0; r < Rows; ++r) {
        for (int c = 0; c < Cols; ++c) {
            tiles[r][c].reset();
        }
    }
    currentRow = 0;
    currentCol = 0;
    lastAnimatedRow = -1;
    gameOver = false;
    won = false;
}

void Board::forceGameOver() {
    gameOver = true;
}
