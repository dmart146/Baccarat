#include <iostream>
#include "cards.h"
#include <regex>

//plays a game of baccarat
int play(Cards &deck, std::string &result);
//converts card string to number value for evaluation
int convert(const  std::string &card);

int main(int argc, char **argv) {
    Cards deck;
    int money = 1000; //starting money
    if(argc > 2) {
        deck = Cards(std::stoi(argv[1]));
        deck.shuffle();
        std::cout << "Created and shuffled a shoe with " << argv[1] << " decks.\n";
        money = std::stoi(argv[2]);
    }
    else if(argc > 1) {
        deck = Cards(std::stoi(argv[1]));
        deck.shuffle();
        std::cout << "Created and shuffled a shoe with " << argv[1] << " decks.\n";
    } else {
        deck.shuffle();
        std::cout << "Created and shuffled a single deck.\n";
    }
    std::cout << "Starting money set to " << money << "\n";

    //burn the shoe
    std::string burnedCard = deck.draw();
    std::cout << "Burned card: " << burnedCard << "\n";
    if(convert(burnedCard) == 0) {
        for(int i=0; i<10; i++) {
            deck.draw();
            std::cout << "Burned a Card\n";
        }
    }
    for(int i=0; i<(convert(burnedCard)-1); i++) {
        deck.draw();
        std::cout << "Burned a Card\n";
    }
    std::cout << "Starting Baccarat Game!\n";


    std::string input;
    int amount = 0;
    while (input != "quit" || amount != -1) {
        std::cout << "Enter bet (B or P followed by amount): ";
        std::getline(std::cin, input);
        std::cout << "Received input: " << input << "\n";
        
        std::smatch m;
        std::regex re(R"(^\s*([bp])\s+(\d+)\s*$)", std::regex_constants::icase);
        if (!std::regex_match(input, m, re)) {
            std::cout << "Invalid input. Expected 'B <amount>' or 'P <amount>'.\n";
            return 1;
        }
        std::string side = m[1].str();           // "B" or "P" (case as entered)
        amount = std::stoi(m[2].str());      // bet amount

        std::cout << "Parsed bet: " << side << " " << amount << "\n";

        // proceed with game
        std::string result = "";
        int win = play(deck, result);
        //introduce a switch to evaluate wins and take from money based on win
        std::cout << result << "\n";
    }

    
    return 0;
}

int convert(const  std::string &card) {
    std::string rank = card.substr(0, card.length() - 3); // Adjusted to handle 10 correctly
    if (rank == "A") return 1;
    if (rank == "K" || rank == "Q" || rank == "J" || rank == "10") return 0;
    return std::stoi(rank);
}

//converts totals to winner value
int eval(int playerTotal, int bankerTotal, bool player3, bool banker3) {
    int playerMod = playerTotal % 10;
    int bankerMod = bankerTotal % 10;
    if (playerMod > bankerMod){
        if (player3 && playerMod==8) return 4; //player win with panda
        return 1;
    } //player win
    if (bankerMod > playerMod){
        if (banker3 && bankerMod==8) return 5; //banker win with dragon
        return 2;
    } //banker win
    return 3; //tie
}

//change to input a reference to a string that gets modified by current output
//actual return now becomes int of who won
//0 = error 1 = player, 2 = banker, 3 = tie, 4 = panda, 5 = dragon
int play(Cards &deck, std::string &result) {
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
    bool playerThirdCard = false;
    bool bankerThirdCard = false;

    //make the string to be returned
    result = std::string("Player: ") + playerCard1 + " " + playerCard2;

    //naturals
    if (playerTotal % 10 >= 8 || bankerTotal % 10 >= 8) {
        result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2;
        return eval(playerTotal, bankerTotal, playerThirdCard, bankerThirdCard);
    }
    if (playerTotal % 10 <= 5) {
        playerCard3 = deck.draw();
        playerThirdCard = true;
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
            bankerThirdCard = true;
            bankerTotal += convert(bankerCard3);
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
            return eval(playerTotal, bankerTotal, playerThirdCard, bankerThirdCard);
        }
        else if (bankerTotal % 10 == 3 && playerThirdCardValue != 8) {
            bankerCard3 = deck.draw();
            bankerThirdCard = true;
            bankerTotal += convert(bankerCard3);
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
            return eval(playerTotal, bankerTotal, playerThirdCard, bankerThirdCard);
        }
        else if (bankerTotal % 10 == 4 && (playerThirdCardValue >= 2 && playerThirdCardValue <= 7)) {
            bankerCard3 = deck.draw();
            bankerThirdCard = true;
            bankerTotal += convert(bankerCard3);
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
            return eval(playerTotal, bankerTotal, playerThirdCard, bankerThirdCard);
        }
        else if (bankerTotal % 10 == 5 && (playerThirdCardValue >= 4 && playerThirdCardValue <= 7)) {
            bankerCard3 = deck.draw();
            bankerThirdCard = true;
            bankerTotal += convert(bankerCard3);
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
            return eval(playerTotal, bankerTotal, playerThirdCard, bankerThirdCard);
        }
        else if (bankerTotal % 10 == 6 && (playerThirdCardValue == 6 || playerThirdCardValue == 7)) {
            bankerCard3 = deck.draw();
            bankerThirdCard = true;
            bankerTotal += convert(bankerCard3);
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
            return eval(playerTotal, bankerTotal, playerThirdCard, bankerThirdCard);
        }
        //does not draw on 7 at all
        result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2;
        return eval(playerTotal, bankerTotal, playerThirdCard, bankerThirdCard);
    }
    else {
        if (bankerTotal % 10 <= 5) {
            bankerCard3 = deck.draw();
            bankerThirdCard = true;
            bankerTotal += convert(bankerCard3);
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
        } else {
            result += std::string(" Banker: ") + bankerCard1 + " " + bankerCard2;
        }
    }
    return eval(playerTotal, bankerTotal, playerThirdCard, bankerThirdCard);
    
}