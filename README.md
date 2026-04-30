# Wordle Game

A Wordle clone built with C++17 and SFML 3.

## Features

- Classic 6×5 Wordle gameplay with a clean on-screen QWERTY keyboard
- Smooth tile animations: pop on input, flip reveal on submission, shake on invalid word
- Duplicate-aware word comparison (handles repeated letters correctly)
- Built-in dictionary with random target word selection
- Real-time keyboard color updates showing letter status

## Dependencies

- CMake 3.16 or higher
- C++17 compatible compiler
- SFML 3 (Graphics, Window, System modules)

## Building

```bash
cmake -B build
cmake --build build
```

The build system automatically copies the `assets/` directory into the build output.

## Running

```bash
./build/WordleGame
```

If no system font is found, place a `.ttf` file at `assets/fonts/DejaVuSans.ttf`.

## Controls

| Key | Action |
|-----|--------|
| A–Z | Enter a letter |
| Backspace | Delete last letter |
| Enter | Submit guess |

## Project Structure

```
├── src/
│   ├── main.cpp        # Entry point
│   ├── Game.hpp/cpp    # Main loop, rendering, input handling
│   ├── Board.hpp/cpp   # 6×5 grid and word logic
│   ├── Tile.hpp/cpp    # Individual tile with animations
│   └── Dictionary.hpp/cpp  # Word list and validation
├── assets/
│   ├── data/words.txt  # Dictionary of 5-letter words
│   └── fonts/          # Optional fallback font directory
├── CMakeLists.txt
└── CLAUDE.md           # Development notes for Claude Code
```

## Colors

| State | Color |
|-------|-------|
| Correct | `#538d4e` |
| Present | `#b59f3b` |
| Wrong | `#3a3a3c` |
