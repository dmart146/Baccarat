// cards.h — Cards deck class for simple drawing
#ifndef CARDS_H
#define CARDS_H

#include <string>
#include <vector>
#include <random>
#include <iostream>

class Cards {
public:
    // Construct a standard 52-card deck
    Cards();
    Cards(int decks);

    // Draw the top card from the deck and return it as a human-readable string.
    // If the deck is empty, returns an empty string.
    std::string draw();

    // Shuffle the remaining cards in the deck
    void shuffle();

    // Number of remaining cards
    size_t size() const;

    void PrintDeck() const;

private:
    std::vector<std::string> deck_;
    std::mt19937 rng_;
};

#endif // CARDS_H
