#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "AudioHelper.hpp"
#include "SlowBullet.hpp"
#include "Group.hpp"
#include "MachineGunTurret.hpp"
#include "PlayScene.hpp"
#include "Point.hpp"
#include "ShootEffectBurst.hpp"


const int MachineGunTurret::Price = 60;
MachineGunTurret::MachineGunTurret(float x, float y) :
    // TODO 3 (1/5): You can imitate the 2 files: 'PlugGunTurret.hpp', 'PlugGunTurret.cpp' to create a new turret.
    Turret("play/tower-base.png", "play/turret-1.png", x, y, 170, Price, 0.5) { //added

    // Move center downward, since we the turret head is slightly biased upward
    Anchor.y += 8.0f / GetBitmapHeight();
}
void MachineGunTurret::CreateBullet() {
    int turretLvl = this->lvl;
    const float BulletSpace = 12.0f;
    Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
    float rotation = atan2(diff.y, diff.x);
    Engine::Point normalized = diff.Normalize();
    // Change bullet position to the front of the gun barrel.

    //Use bullet spacing to calculate the offset for the bullets. Will be 0 at turretlvl == 1.
    for (int i = 0; i < turretLvl; i++) {
        float AngleOffset = (i - (turretLvl - 1) / 2.0f) * BulletSpace;
        Engine::Point Perpendicular = Engine::Point(-normalized.y, normalized.x);
        Engine::Point BulletPos = Position + normalized * 36 + Perpendicular * AngleOffset;
        getPlayScene()->BulletGroup->AddNewObject(new SlowBullet( BulletPos, diff, rotation, this));
    }
    getPlayScene()->EffectGroup->AddNewObject(new ShootEffectBurst(this->Position + normalized * 36));
    AudioHelper::PlayAudio("gun.wav");
}
int MachineGunTurret::GetTurretID() {
    return 1;
}

void MachineGunTurret::UpTurret(std::string imgNewTurret) {
    this->CollisionRadius = 220; //increase its fire range slightly.
    this->coolDown = 0.4; //increase its firerate slightly.
    Engine::Sprite upgradeTurret(imgNewTurret, Position.x, Position.y);
    std::swap(Sprite::bmp, upgradeTurret.bmp);
}