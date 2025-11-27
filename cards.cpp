// cards.cpp — Implementation of Cards
#include "cards.h"

#include <algorithm>
#include <chrono>
#include <random>

using namespace std;

Cards::Cards() {
    static const vector<string> suits = {"♣", "♦", "♥", "♠"};
    static const vector<string> ranks = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};

    for (const auto &s : suits) {
        for (const auto &r : ranks) {
            deck_.push_back(r + s);
        }
    }

}
Cards::Cards(int decks) {
    for (int i = 0; i < decks; ++i) {
        Cards singleDeck;
        deck_.insert(deck_.end(), singleDeck.deck_.begin(), singleDeck.deck_.end());
    }
}

void Cards::shuffle() {
    uint64_t seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    mt19937 rng(static_cast<uint32_t>(seed & 0xffffffffu));

    std::shuffle(deck_.begin(), deck_.end(), rng);
}

std::string Cards::draw() {
    if (deck_.empty()) return std::string();
    string card = deck_.back();
    deck_.pop_back();
    return card;
}

size_t Cards::size() const {
    return deck_.size();
}

void Cards::PrintDeck() const {
    for (const auto &card : deck_) {
        std::cout << card << " ";
    }
    std::cout << std::endl;
}
