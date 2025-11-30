#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include <iostream>


class Player {
public:
    Player();

    void updateBets(std::string bet, int balance, int dragonBet, int pandaBet);
    
private:
    std::string baseBet;
    int baseBalance;
    bool dragon;
    bool panda; 
    int dragonBalance;
    int pandaBalance;
    int payout = 0;
};

#endif // PLAYER_H 