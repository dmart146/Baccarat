#include <iostream>
#include "cards.h"

std::string play(Cards &deck);

int main(int argc, char **argv) {
    if(argc > 1) {
        Cards deck(std::stoi(argv[1]));
        deck.shuffle();
        std::cout << "Created and shuffled a shoe with " << argv[1] << " decks.\n";
    } else {
        Cards deck;
        deck.shuffle();
        std::cout << "Created and shuffled a single deck.\n";
    }

    std::string input;
    std::cout << "Enter bet (B or P followed by amount): ";
    std::cin >> input;

input_start:
    if (input.length() > 1) {
        char choice = input[0];
        int amount = std::stoi(input.substr(1));
        if (choice == 'B' || choice == 'P') {
            std::cout << "Bet: " << choice << " for $" << amount << "\n";
        }
        else{
            std::cout << "Invalid bet choice.\n";
            goto input_start;
        }
    }
    
    return 0;
}

int convert(const  std::string &card) {
    std::string rank = card.substr(0, card.length() - 3); // Adjusted to handle 10 correctly
    if (rank == "A") return 1;
    if (rank == "K" || rank == "Q" || rank == "J" || rank == "10") return 0;
    return std::stoi(rank);
}

//change to input a reference to a string that gets modified by current output
//actual return now becomes int of who won
std::string play(Cards &deck) {
    int playerTotal = 0;
    int bankerTotal = 0;
    //pull for player, then banker, then player again, banker again, then decide third cards
    std::string playerCard1 = deck.draw();
    playerTotal += convert(playerCard1);
    std::string bankerCard1 = deck.draw();
    bankerTotal += convert(bankerCard1);
    //first cards drawn

    std::string playerCard2 = deck.draw();
    playerTotal += convert(playerCard2);
    std::string bankerCard2 = deck.draw();
    bankerTotal += convert(bankerCard2);
    //second cards drawn

    std::string playerCard3 = "";
    std::string bankerCard3 = "";

    //make the string to be returned
    std::string result = std::string("Player: ") + playerCard1 + " " + playerCard2;

    //naturals
    if (playerTotal % 10 >= 8 || bankerTotal % 10 >= 8) {
        result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2;
        return result;
    }
    if (playerTotal % 10 <= 5) {
        playerCard3 = deck.draw();
        playerTotal += convert(playerCard3);
        result += " ";
        result += playerCard3;
    }

    //player third card rule

    if (!playerCard3.empty()) {
        //banker third card rule based on playerCard3
        int playerThirdCardValue = convert(playerCard3);
        if (bankerTotal % 10 <= 2) {
            bankerCard3 = deck.draw();
            bankerTotal += convert(bankerCard3);
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
            return result;
        }
        else if (bankerTotal % 10 == 3 && playerThirdCardValue != 8) {
            bankerCard3 = deck.draw();
            bankerTotal += convert(bankerCard3);
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
            return result;
        }
        else if (bankerTotal % 10 == 4 && (playerThirdCardValue >= 2 && playerThirdCardValue <= 7)) {
            bankerCard3 = deck.draw();
            bankerTotal += convert(bankerCard3);
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
            return result;
        }
        else if (bankerTotal % 10 == 5 && (playerThirdCardValue >= 4 && playerThirdCardValue <= 7)) {
            bankerCard3 = deck.draw();
            bankerTotal += convert(bankerCard3);
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
            return result;
        }
        else if (bankerTotal % 10 == 6 && (playerThirdCardValue == 6 || playerThirdCardValue == 7)) {
            bankerCard3 = deck.draw();
            bankerTotal += convert(bankerCard3);
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
            return result;
        }
        //does not draw on 7 at all
        result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2;
        return result;
    }
    else {
        if (bankerTotal % 10 <= 5) {
            bankerCard3 = deck.draw();
            bankerTotal += convert(bankerCard3);
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
        } else {
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2;
        }
    }
    return result;
    
}