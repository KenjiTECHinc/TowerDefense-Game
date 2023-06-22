#ifndef TURRET_HPP
#define TURRET_HPP
#include <allegro5/base.h>
#include <list>
#include <string>

#include "Sprite.hpp"

class Enemy;
class PlayScene;

class Turret: public Engine::Sprite {
protected:
    int const_price;
    float coolDown;
    float reload = 1; //default: 0, can control delay time after placing.
    float rotateRadian = 2 * ALLEGRO_PI;
    Sprite imgBase;
    std::list<Turret*>::iterator lockedTurretIterator;
    PlayScene* getPlayScene();
    // Reference: Design Patterns - Factory Method.
    virtual void CheckBullet(bool inSight, float reload);
    virtual void CreateBullet() = 0;

public:
    int Bullet_cnt = 0;
    int price;
    int lvl = 1;
    bool Enabled = true;
    bool Preview = false;
    Enemy* Target = nullptr;
    Turret(std::string imgBase, std::string imgTurret, float x, float y, float radius, int price, float coolDown); //added turret2
    void Update(float deltaTime) override;
    void Draw() const override;
	int GetPrice(int request) const;
    virtual int GetTurretID();
    virtual void UpTurret(std::string imgNewTurret);
    virtual int GetTurretBullet();
};
#endif // TURRET_HPP
