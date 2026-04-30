#pragma once

#include <SFML/Graphics.hpp>

class Tile {
public:
    enum class State {
        Empty,
        Wrong,
        Present,
        Correct
    };

    Tile();

    void setPosition(float x, float y);
    void setLetter(char c);
    void setState(State s);
    void triggerPop();
    void triggerFlip(State resultState);
    void triggerShake();

    char getLetter() const { return letter; }
    State getState() const { return currentState; }
    bool isAnimating() const;

    void update(float dt);
    void draw(sf::RenderWindow& window, const sf::Font& font) const;

    static constexpr float Size = 62.f;
    static constexpr float Margin = 6.f;

private:
    sf::Vector2f basePosition;
    char letter = ' ';
    State currentState = State::Empty;
    State targetState = State::Empty;

    // Pop animation
    bool popping = false;
    float popTime = 0.f;
    static constexpr float PopDuration = 0.15f;

    // Flip animation
    bool flipping = false;
    float flipTime = 0.f;
    static constexpr float FlipDuration = 0.5f;

    // Shake animation
    bool shaking = false;
    float shakeTime = 0.f;
    static constexpr float ShakeDuration = 0.4f;
    static constexpr float ShakeIntensity = 8.f;

    float scaleY = 1.f;
    float currentScale = 1.f;
    float shakeOffset = 0.f;

    sf::Color getColorForState(State s) const;
    sf::Color getBorderColor() const;
};
