#pragma once

#include <string>
#include <unordered_set>
#include <vector>
#include <random>

class Dictionary {
public:
    bool loadFromFile(const std::string& filename);
    bool isValidWord(const std::string& word) const;
    std::string getRandomWord() const;

private:
    std::unordered_set<std::string> words;
    std::vector<std::string> wordList;
    mutable std::mt19937 rng{std::random_device{}()};
};
