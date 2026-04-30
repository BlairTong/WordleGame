#include "Tile.hpp"
#include <cmath>

Tile::Tile() = default;

void Tile::setPosition(float x, float y) {
    basePosition = {x, y};
}

void Tile::setLetter(char c) {
    letter = static_cast<char>(std::toupper(c));
    triggerPop();
}

void Tile::setState(State s) {
    currentState = s;
}

void Tile::triggerPop() {
    popping = true;
    popTime = 0.f;
}

void Tile::triggerFlip(State resultState) {
    flipping = true;
    flipTime = 0.f;
    targetState = resultState;
}

void Tile::triggerShake() {
    shaking = true;
    shakeTime = 0.f;
}

bool Tile::isAnimating() const {
    return popping || flipping || shaking;
}

void Tile::update(float dt) {
    if (popping) {
        popTime += dt;
        if (popTime >= PopDuration) {
            popping = false;
            currentScale = 1.f;
        } else {
            float t = popTime / PopDuration;
            if (t < 0.5f) {
                currentScale = 0.8f + (0.3f * (t / 0.5f));
            } else {
                currentScale = 1.1f - (0.1f * ((t - 0.5f) / 0.5f));
            }
        }
    }

    if (flipping) {
        flipTime += dt;
        if (flipTime >= FlipDuration) {
            flipping = false;
            scaleY = 1.f;
            currentState = targetState;
        } else {
            float t = flipTime / FlipDuration;
            scaleY = std::cos(t * 3.14159265f);
            if (scaleY < 0.f) {
                currentState = targetState;
                scaleY = -scaleY;
            }
        }
    }

    if (shaking) {
        shakeTime += dt;
        if (shakeTime >= ShakeDuration) {
            shaking = false;
            shakeOffset = 0.f;
        } else {
            shakeOffset = std::sin(shakeTime * 30.f) * ShakeIntensity * (1.f - shakeTime / ShakeDuration);
        }
    }
}

sf::Color Tile::getColorForState(State s) const {
    switch (s) {
        case State::Correct: return sf::Color(0x53, 0x8d, 0x4e);
        case State::Present: return sf::Color(0xb5, 0x9f, 0x3b);
        case State::Wrong:   return sf::Color(0x3a, 0x3a, 0x3c);
        case State::Empty:   return sf::Color::Transparent;
    }
    return sf::Color::Transparent;
}

sf::Color Tile::getBorderColor() const {
    if (currentState == State::Empty && letter == ' ') return sf::Color(0xd3, 0xd6, 0xda);
    if (currentState == State::Empty) return sf::Color(0x87, 0x8a, 0x8c);
    return getColorForState(currentState);
}

void Tile::draw(sf::RenderWindow& window, const sf::Font& font) const {
    sf::RectangleShape rect({Size, Size});
    float scaledSize = Size * currentScale;
    float offset = (Size - scaledSize) * 0.5f;
    rect.setSize({scaledSize, Size * scaleY});
    rect.setPosition({
        basePosition.x + offset + shakeOffset,
        basePosition.y + offset + (Size - Size * scaleY) * 0.5f
    });

    if (currentState != State::Empty) {
        rect.setFillColor(getColorForState(currentState));
        rect.setOutlineThickness(0);
    } else {
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineThickness(2);
        rect.setOutlineColor(getBorderColor());
    }

    window.draw(rect);

    if (letter != ' ') {
        sf::Text text(font, std::string(1, letter), 32);
        text.setFillColor(sf::Color(0x1a, 0x1a, 0x1a));
        text.setStyle(sf::Text::Bold);

        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
        text.setPosition({
            basePosition.x + Size / 2.f + shakeOffset,
            basePosition.y + Size / 2.f
        });
        window.draw(text);
    }
}
