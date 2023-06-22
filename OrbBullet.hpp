#ifndef ORBBULLET_HPP
#define ORBBULLET_HPP
#include <string>

//#include "Bullet.hpp"
#include "Sprite.hpp"
#include "OrbTurret.hpp" //include header file

class Enemy;
class PlayScene;
class Turret;
namespace Engine {
    struct Point;
}  // namespace Engine

class OrbBullet : public Engine::Sprite {
protected:
    float speed;
    float damage;
    OrbTurret* parent; //For accessing OrbTurret's map. [Originally >> Turret* parent]
    PlayScene* getPlayScene();

    //----NEW----
    //BUG ZONE: Initial issue = bullet move to new parent position
    //BUG ZONE: status = resolved --> move it as attribute of individual instances.
    float parentPositionX;
    float parentPositionY;
    float OrbRadius;

public:
    int Bullet_ID; //for identifying itself in the parent's map.
    Enemy* Target = nullptr;
    explicit OrbBullet(float bulletSpeed, float bulletDamage, Engine::Point position, Engine::Point forwardDirection, float rotation, OrbTurret* parent, float orbitRadius, int ID); //added ID
    virtual void OnExplode(Enemy* enemy);
    void Update(float deltaTime) override;
    void SelfDestruct(); //Fix Attempt
};
#endif // WOODBULLET_HPP
