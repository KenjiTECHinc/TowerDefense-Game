#ifndef DICECARRIERENEMY_HPP
#define DICECARRIERENEMY_HPP
#include "Enemy.hpp"
#include "Sprite.hpp"
#include "Point.hpp"


class Turret; 

class DiceCarrierEnemy : public Enemy {
public:
    //int extraLives = 1;
    DiceCarrierEnemy(int x, int y);
    void OnExplode() override; //override virtual function from Enemy class. ^^
    void Hit(float damage) override;
};
#endif // DICECARRIERENEMY_HPP
