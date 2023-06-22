#ifndef DICECARRIERENEMY2_HPP
#define DICECARRIERENEMY2_HPP
#include "Enemy.hpp"
#include "Sprite.hpp"
#include "Point.hpp"


class Turret;

class DiceCarrierEnemy2 : public Enemy {
protected:
    int hit_cnt = 0;
public:
    int extraLives;
    int countDown = 5;
    DiceCarrierEnemy2(int x, int y);
    void OnExplode() override; //override virtual function from Enemy class. ^^
    void Hit(float damage) override;
    //void Update(float deltaTime) override;
};
#endif // DICECARRIERENEMY_HPP
