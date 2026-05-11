# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Run

Dependencies: CMake 3.16+, C++17 compiler, SFML 3 (Graphics, Window, System).

```bash
cmake -B build
cmake --build build
./build/WordleGame
```

The `CMakeLists.txt` copies the `assets/` directory into the build output automatically. If SFML is not installed system-wide, install it first (e.g., `brew install sfml` on macOS, `sudo apt-get install libsfml-dev` on Ubuntu).

**SFML 3 API notes:** The code uses SFML 3 patterns: `std::optional<sf::Event>` from `pollEvent()`, `event.is<sf::Event::Closed>()` / `event.getIf<sf::Event::TextEntered>()`, `sf::Keyboard::Key::Enter`, `sf::Text(font, string, size)` constructor (no default ctor), `setPosition({x, y})` with brace-init, and `sf::FloatRect` members `position` / `size` instead of `left` / `top` / `width` / `height`.

## Font Loading

The game requires a TrueType font. `Game::loadFont()` tries several system font paths, then falls back to `assets/fonts/DejaVuSans.ttf`. If no font loads, the game exits with an error. Place a `.ttf` file at that path if the system has no suitable default fonts.

## High-Level Architecture

### Game Loop and Ownership (`Game.hpp/cpp`)

`Game` owns the `sf::RenderWindow`, `Board`, and `Dictionary`. The loop lives in `Game::run()` and delegates to `processEvents()`, `update(float dt)`, and `render()`. Input is ignored while `waitingForAnimation` is true (set after a valid Enter submission, cleared when the row finishes animating) or when `board.isGameOver()` is true.

Window size: 600×800 px, framerate cap at 60 fps.

Input handling:
- `sf::Event::TextEntered` for A–Z letters (both uppercase and lowercase accepted, converted to lowercase).
- `sf::Event::KeyPressed` for Backspace and Enter.

`Game` also maintains `keyboardColors`, a `map<char, KeyState>` used to render the on-screen QWERTY keyboard with colors updated after each submitted guess (priority: Correct > Present > Wrong > Empty).

### Grid and Word Logic (`Board.hpp/cpp`)

`Board` manages a fixed `6×5` grid of `Tile` objects. It tracks the current row and column for input.

- `addLetter(char)` writes to the current cell and advances the column.
- `removeLetter()` decrements the column and clears the cell.
- `applyResult(const std::string& target)` computes the result states for the current row and triggers flip animations, then advances the row.

**Word comparison** is implemented in `getRowResult()` with a two-pass algorithm to correctly handle duplicate letters (e.g., target `ABBEY`, guess `BABES` marks each `B` accurately without double-counting).

### Animations (`Tile.hpp/cpp`)

Each `Tile` handles its own animations using delta time:
- **Pop**: triggered on letter entry. Scale animates `0.8 → 1.1 → 1.0` over `0.15s`.
- **Flip/Reveal**: triggered by `Board::applyResult()`. `scaleY` uses `cos(t * π)` so the tile appears to rotate around its horizontal center. At `scaleY < 0` the background color switches to the result color. Duration is `0.5s` per tile.
- **Shake**: triggered when an invalid word is submitted. Horizontal offset follows `sin(t × 30) × 8 × (1 − t/duration)` decaying over `0.4s`.

`Tile::isAnimating()` returns true if any animation is active; `Board::isRowAnimating()` queries `lastAnimatedRow` to block input during reveals.

### Dictionary (`Dictionary.hpp/cpp`)

Loads `assets/data/words.txt` (one uppercase 5-letter word per line) into an `std::unordered_set` for O(1) `isValidWord()` checks. `getRandomWord()` selects from a parallel `std::vector` using `std::mt19937`.

### UI Overlays

- **Shake message**: When an invalid word is submitted, "Not in word list" appears near the top of the screen for 1.2s (`shakeMessageTimer`).
- **Game over overlay**: When `board.isGameOver()` is true, a semi-transparent overlay covers the screen showing "You Win!" or "Game Over" along with the target word.

### Colors

| Usage | Hex |
|-------|-----|
| Background (window clear) | `#f5f5f0` |
| Tile — Correct | `#538d4e` |
| Tile — Present | `#b59f3b` |
| Tile — Wrong | `#3a3a3c` |
| Tile — Empty (unfilled border) | `#d3d6da` (no letter) / `#878a8c` (has letter) |
| Keyboard — uncolored key | `#d3d6da` |
| Keyboard — key text | `#1a1a1a` |
| Tile text | `#1a1a1a` |

Tile and keyboard colors are hard-coded in `Tile::getColorForState()`, `Tile::getBorderColor()`, `Game::drawKeyboard()`, and `Game::render()`.

### Asset Layout

```
assets/
├── data/words.txt    # Dictionary of 5-letter words
└── fonts/            # Optional: place DejaVuSans.ttf here
```
