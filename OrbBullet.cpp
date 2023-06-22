#include <allegro5/base.h>
#include <random>
#include <cmath>
#include <string>
#include <iostream>
using namespace std;

#include "DirtyEffect.hpp"
#include "Enemy.hpp"
#include "GameEngine.hpp"
#include "OrbBullet.hpp"
#include "Group.hpp"
#include "PlayScene.hpp"
#include "Point.hpp"
#include "OrbTurret.hpp" //include header file
#include "IObject.hpp"
#include "IScene.hpp"
#include "Sprite.hpp"
#include "Collider.hpp"
#include "OrbTurret.hpp"

class OrbTurret;
//class Turret;
//float parentPositionX, parentPositionY, OrbRadius;
PlayScene* OrbBullet::getPlayScene() {
    return dynamic_cast<PlayScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
}

//void OrbBullet::OnExplode(Enemy* enemy) {}

OrbBullet::OrbBullet(float bulletSpeed, float bulletDamage, Engine::Point position, Engine::Point forwardDirection, float rotation, OrbTurret* parent, float orbitRadius, int ID) :
    Sprite("play/bullet-11.png", position.x, position.y), speed(bulletSpeed), damage(bulletDamage), parent(parent), Bullet_ID(ID) { //added bullet ID.
    Velocity = forwardDirection.Normalize() * bulletSpeed;
    Rotation = rotation;
    CollisionRadius = 4;
    this->OrbRadius = orbitRadius;
    this->parentPositionX = parent->Position.x;
    this->parentPositionY = parent->Position.y;
    

    // TODO 3 (2/5): You can imitate the 2 files: 'WoodBullet.hpp', 'WoodBullet.cpp' to create a new bullet.
}
void OrbBullet::Update(float deltaTime) {
    Sprite::Update(deltaTime);
    PlayScene* Scene = getPlayScene();
    if (!Scene) {
        return;
    }
    float orbRadius = OrbRadius;
    float orbSpeed = speed / orbRadius;
    Rotation += orbSpeed * deltaTime;
    float newPosX = this->parentPositionX + orbRadius * cos(Rotation);
    float newPosY = this->parentPositionY + orbRadius * sin(Rotation);
    Position = Engine::Point(newPosX, newPosY);

    for (auto& it : Scene->EnemyGroup->GetObjects()) {
        Enemy* enemy = dynamic_cast<Enemy*>(it);
        if (!enemy->Visible) {
            continue;
        }
        if (Engine::Collider::IsCircleOverlap(Position, CollisionRadius, enemy->Position, enemy->CollisionRadius)) {
            dynamic_cast<OrbTurret*>(parent)->Bullet_cnt--;
            OnExplode(enemy);
            enemy->Hit(damage);
            //-------BUG ATTEMPT-------- // IDEA: remove the object from parent's map reference when deleted
            map<int, OrbBullet*>::iterator it;
            it = parent->BulletID.find(Bullet_ID);
            if (it != parent->BulletID.end()) parent->BulletID.erase(it);
            cout << "This is the <map> size: " << parent->BulletID.size() << endl;
            //-------END ATTEMPT--------
            getPlayScene()->BulletGroup->RemoveObject(objectIterator); //remove itself after.
            return;
        }
    }/* DOUBT: Unecessary / Critical issue to game excecution!?
    if (!Engine::Collider::IsRectOverlap(Position - Size / 2, Position + Size / 2, Engine::Point(0, 0), PlayScene::GetClientSize())) {
        dynamic_cast<OrbTurret*>(parent)->Bullet_cnt--;
        getPlayScene()->BulletGroup->RemoveObject(objectIterator);
    }*/
}

void OrbBullet::OnExplode(Enemy* enemy) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(2, 5);
    getPlayScene()->GroundEffectGroup->AddNewObject(new DirtyEffect("play/dirty-3.png", dist(rng), enemy->Position.x, enemy->Position.y));
}

//PURPOSE: For self destruction, remove itself after erasing itself in map.
void OrbBullet::SelfDestruct() {
    //-------BUG ATTEMPT--------
    map<int, OrbBullet*>::iterator it;
    it = parent->BulletID.find(Bullet_ID);
    if (it != parent->BulletID.end()) parent->BulletID.erase(it);
    //-------END ATTEMPT--------
    getPlayScene()->BulletGroup->RemoveObject(objectIterator);
    return; 
}