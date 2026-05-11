#include "player.h"
using namespace std;

Player::Player() : baseBet(""), baseBalance(0), dragon(false), panda(false), dragonBalance(0), pandaBalance(0), payout(0) {}
void Player::updateBets(std::string bet, int balance, int dragonBet, int pandaBet) {
    baseBet = bet;
    baseBalance = balance;
    dragon = (dragonBet > 0);
    panda = (pandaBet > 0);
    dragonBalance = dragonBet;
    pandaBalance = pandaBet;
}

int Player::calculatePayout(int result) const {
    int total = -(baseBalance + dragonBalance + pandaBalance);

    bool isPlayerBet = (baseBet == "P" || baseBet == "p");
    bool isBankerBet = (baseBet == "B" || baseBet == "b");

    if (result == 3) {
        // Tie: main bet pushes (returned)
        total += baseBalance;
    } else if ((result == 1 || result == 4) && isPlayerBet) {
        // Player wins (or Panda 8): 1:1
        total += baseBalance * 2;
    } else if ((result == 2 || result == 5) && isBankerBet) {
        // Banker wins (or Dragon 7): 0.95:1 after 5% commission
        total += baseBalance + static_cast<int>(baseBalance * 0.95);
    }

    // Dragon 7 side bet: banker 3-card 7 → 40:1
    if (result == 5 && dragon) {
        total += dragonBalance * 41;
    }

    // Panda 8 side bet: player 3-card 8 → 25:1
    if (result == 4 && panda) {
        total += pandaBalance * 26;
    }

    return total;
}