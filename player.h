#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include <iostream>


class Player {
public:
    Player();

    void updateBets(std::string bet, int balance, int dragonBet, int pandaBet);

    // Returns net payout: positive = win, negative = loss.
    // result codes: 1=player, 2=banker, 3=tie, 4=panda8, 5=dragon7
    int calculatePayout(int result) const;

    std::string getBet() const { return baseBet; }

private:
    std::string baseBet;
    int baseBalance;
    bool dragon;
    bool panda; 
    int dragonBalance;
    int pandaBalance;
};

#endif // PLAYER_H 