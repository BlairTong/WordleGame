#include "Dictionary.hpp"
#include <fstream>
#include <iostream>

bool Dictionary::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open dictionary: " << filename << std::endl;
        return false;
    }

    std::string word;
    while (file >> word) {
        if (word.size() == 5) {
            for (auto& c : word) c = static_cast<char>(std::toupper(c));
            words.insert(word);
            wordList.push_back(word);
        }
    }
    return !wordList.empty();
}

bool Dictionary::isValidWord(const std::string& word) const {
    std::string upper;
    upper.reserve(word.size());
    for (char c : word) upper.push_back(static_cast<char>(std::toupper(c)));
    return words.find(upper) != words.end();
}

std::string Dictionary::getRandomWord() const {
    if (wordList.empty()) return "";
    std::uniform_int_distribution<size_t> dist(0, wordList.size() - 1);
    return wordList[dist(rng)];
}
