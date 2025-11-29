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