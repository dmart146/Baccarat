#include <iostream>
#include <string>
#include <regex>
#include "cards.h"
#include "player.h"

int play(Cards &deck, std::string &result);
int convert(const std::string &card);
int eval(int playerTotal, int bankerTotal, bool player3, bool banker3);

int main(int argc, char **argv) {
    // Flush after every cout so prompts appear immediately when piped (e.g. web bridge).
    std::cout << std::unitbuf;

    int numDecks = 1;
    int money = 1000;

    if (argc > 2) {
        numDecks = std::stoi(argv[1]);
        money    = std::stoi(argv[2]);
    } else if (argc > 1) {
        numDecks = std::stoi(argv[1]);
    }

    Cards deck(numDecks);
    deck.shuffle();
    std::cout << "Shuffled shoe: " << numDecks << " deck(s).\n";
    std::cout << "Starting balance: $" << money << "\n";

    // Burn the first card; burn that many additional cards per casino rules
    std::string burnCard = deck.draw();
    std::cout << "Burned: " << burnCard << "\n";
    int burnCount = convert(burnCard);
    if (burnCount == 0) burnCount = 10;
    for (int i = 0; i < burnCount - 1; i++) deck.draw();

    std::cout << "\n=== Mini-Baccarat ===\n";
    std::cout << "Bet format: B/P <amount> [D <amount>] [PA <amount>]\n";
    std::cout << "  B=Banker  P=Player  D=Dragon7 side(40:1)  PA=Panda8 side(25:1)\n";
    std::cout << "Type 'quit' to cash out.\n\n";

    // Matches: B/P amount, optional D amount, optional PA amount
    std::regex re(
        R"(^\s*([bp])\s+(\d+)(?:\s+d\s+(\d+))?(?:\s+pa\s+(\d+))?\s*$)",
        std::regex_constants::icase
    );
    std::string input;

    while (true) {
        // Reshuffle when fewer than 15 cards remain
        if (static_cast<int>(deck.size()) < 15) {
            std::cout << "[Shoe running low — reshuffling]\n";
            deck = Cards(numDecks);
            deck.shuffle();
            std::string bc = deck.draw();
            int n = convert(bc);
            if (n == 0) n = 10;
            for (int i = 0; i < n - 1; i++) deck.draw();
        }

        std::cout << "Balance: $" << money << "  |  Cards: " << deck.size() << "\n";
        std::cout << "Bet: ";
        if (!std::getline(std::cin, input)) break;

        if (input == "quit") break;

        std::smatch m;
        if (!std::regex_match(input, m, re)) {
            std::cout << "Invalid. Example: 'B 100'  or  'P 100 D 25 PA 10'\n\n";
            continue;
        }

        std::string side  = m[1].str();
        int betAmt        = std::stoi(m[2].str());
        int dragonAmt     = m[3].matched ? std::stoi(m[3].str()) : 0;
        int pandaAmt      = m[4].matched ? std::stoi(m[4].str()) : 0;
        int totalBet      = betAmt + dragonAmt + pandaAmt;

        if (betAmt <= 0) {
            std::cout << "Main bet must be > 0.\n\n";
            continue;
        }
        if (totalBet > money) {
            std::cout << "Not enough funds. Balance: $" << money << "\n\n";
            continue;
        }

        Player player;
        player.updateBets(side, betAmt, dragonAmt, pandaAmt);

        std::string result;
        int outcome = play(deck, result);
        std::cout << "\n" << result << "\n";

        // Outcome labels
        const char* labels[] = {"", "Player wins", "Banker wins", "Tie — push",
                                 "Panda 8 — Player wins!", "Dragon 7 — Banker wins!"};
        std::cout << labels[outcome];
        if (outcome == 4 && pandaAmt > 0) std::cout << "  [Panda side bet pays 25:1!]";
        if (outcome == 5 && dragonAmt > 0) std::cout << "  [Dragon side bet pays 40:1!]";
        std::cout << "\n";

        int net = player.calculatePayout(outcome);
        money += net;

        std::cout << "Net: " << (net >= 0 ? "+" : "") << net
                  << "  |  Balance: $" << money << "\n\n";

        if (money <= 0) {
            std::cout << "You're broke! Game over.\n";
            break;
        }
    }

    std::cout << "Final balance: $" << money << ". Thanks for playing!\n";
    return 0;
}

int convert(const std::string &card) {
    std::string rank = card.substr(0, card.length() - 3);
    if (rank == "A") return 1;
    if (rank == "K" || rank == "Q" || rank == "J" || rank == "10") return 0;
    return std::stoi(rank);
}

// 0=error, 1=player, 2=banker, 3=tie, 4=panda8(player 3-card 8), 5=dragon7(banker 3-card 7)
int eval(int playerTotal, int bankerTotal, bool player3, bool banker3) {
    int pm = playerTotal % 10;
    int bm = bankerTotal % 10;
    if (pm > bm) {
        if (player3 && pm == 8) return 4; // Panda 8
        return 1;
    }
    if (bm > pm) {
        if (banker3 && bm == 7) return 5; // Dragon 7
        return 2;
    }
    return 3; // tie
}

int play(Cards &deck, std::string &result) {
    int playerTotal = 0;
    int bankerTotal = 0;

    std::string playerCard1 = deck.draw(); playerTotal += convert(playerCard1);
    std::string bankerCard1 = deck.draw(); bankerTotal += convert(bankerCard1);
    std::string playerCard2 = deck.draw(); playerTotal += convert(playerCard2);
    std::string bankerCard2 = deck.draw(); bankerTotal += convert(bankerCard2);

    std::string playerCard3 = "";
    std::string bankerCard3 = "";
    bool playerThirdCard = false;
    bool bankerThirdCard = false;

    result = std::string("Player: ") + playerCard1 + " " + playerCard2;

    // Naturals: no third cards drawn
    if (playerTotal % 10 >= 8 || bankerTotal % 10 >= 8) {
        result += std::string("  Banker: ") + bankerCard1 + " " + bankerCard2;
        return eval(playerTotal, bankerTotal, playerThirdCard, bankerThirdCard);
    }

    // Player draws on 0–5
    if (playerTotal % 10 <= 5) {
        playerCard3 = deck.draw();
        playerThirdCard = true;
        playerTotal += convert(playerCard3);
        result += " " + playerCard3;
    }

    if (!playerCard3.empty()) {
        // Banker third card rules when player drew
        int p3v = convert(playerCard3);
        bool bankerDraws = false;
        int bm = bankerTotal % 10;
        if      (bm <= 2)                                    bankerDraws = true;
        else if (bm == 3 && p3v != 8)                        bankerDraws = true;
        else if (bm == 4 && p3v >= 2 && p3v <= 7)            bankerDraws = true;
        else if (bm == 5 && p3v >= 4 && p3v <= 7)            bankerDraws = true;
        else if (bm == 6 && (p3v == 6 || p3v == 7))          bankerDraws = true;
        // bm == 7: never draws

        if (bankerDraws) {
            bankerCard3 = deck.draw();
            bankerThirdCard = true;
            bankerTotal += convert(bankerCard3);
            result += std::string("  Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
        } else {
            result += std::string("  Banker: ") + bankerCard1 + " " + bankerCard2;
        }
    } else {
        // Player stood; banker draws on 0–5
        if (bankerTotal % 10 <= 5) {
            bankerCard3 = deck.draw();
            bankerThirdCard = true;
            bankerTotal += convert(bankerCard3);
            result += std::string("  Banker: ") + bankerCard1 + " " + bankerCard2 + " " + bankerCard3;
        } else {
            result += std::string("  Banker: ") + bankerCard1 + " " + bankerCard2;
        }
    }

    return eval(playerTotal, bankerTotal, playerThirdCard, bankerThirdCard);
}
