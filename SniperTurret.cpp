#include <allegro5/base.h>
#include <cmath>
#include <string>
#include <iostream>

#include "AudioHelper.hpp"
#include "ArmorPierceBullet.hpp"
#include "Group.hpp"
#include "SniperTurret.hpp"
#include "PlayScene.hpp"
#include "Point.hpp"
#include "ShootEffect.hpp"


const int SniperTurret::Price = 100;
SniperTurret::SniperTurret(float x, float y) :
    // TODO 3 (1/5): You can imitate the 2 files: 'PlugGunTurret.hpp', 'PlugGunTurret.cpp' to create a new turret.
    Turret("play/tower-base.png", "play/turret-11.png", x, y, 450, Price, 5) {
    // Move center downward, since we the turret head is slightly biased upward
    Anchor.y += 8.0f / GetBitmapHeight();
}
void SniperTurret::CreateBullet() {
    Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
    //std::cout << "This is diff.x: " << diff.x << ' ' << diff.y << std::endl;
    float rotation = atan2(diff.y, diff.x);
    //std::cout << "This is rotation value: " << rotation << std::endl;
    Engine::Point normalized = diff.Normalize();
    // Change bullet position to the front of the gun barrel.
    getPlayScene()->BulletGroup->AddNewObject(new ArmorPierceBullet(Position + normalized * 36, diff, rotation, this));
    getPlayScene()->EffectGroup->AddNewObject(new ShootEffect(this->Position + normalized * 36));
    AudioHelper::PlayAudio("gun.wav");
}
int SniperTurret::GetTurretID() {
    return 2;
}