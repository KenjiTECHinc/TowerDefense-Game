#ifndef ORBTURRET_HPP
#define ORBTURRET_HPP
#include "Turret.hpp"
#include <allegro5/bitmap.h>
#include <memory>
#include <vector>
#include <map>
using namespace std;

#include "Sprite.hpp"
class Enemy;
class PlayScene;
class OrbBullet;

class OrbTurret : public Turret {
    friend class OrbBullet;
protected:
    int Construct_flag; //Turret status check: is this tower new?
    void CheckBullet(bool inSight, float reload) override;

public:
    int Bullet_cnt =0;
    static const int Price;
    map < int, OrbBullet*> BulletID; //Fix Attempt
    //map < int, OrbBullet*> *BulletID = new map<int, OrbBullet*>(); //Fix Attempt

    OrbTurret(float x, float y , int bullet_num); //edit constructor function
    void CreateBullet() override;
    int GetTurretID() override;
    int GetTurretBullet() override; //return bullet count.
    void DestructBullet(); //Fix Attempt
    //void Update(float deltaTime) override;
};
#endif 