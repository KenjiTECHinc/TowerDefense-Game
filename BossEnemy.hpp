#ifndef BOSSENEMY_HPP
#define BOSSENEMY_HPP
#include "Enemy.hpp"
#include "Sprite.hpp"
#include "Point.hpp"


class Turret;

class BossEnemy : public Enemy {
protected:
    float cooldown = 15;
public:
    int extraLives;
    BossEnemy(int x, int y);
    void OnExplode() override; //override virtual function from Enemy class. ^^
    void Hit(float damage) override;
    void Update(float deltaTime) override;

};
#endif // DICECARRIERENEMY_HPP
