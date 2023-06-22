#include <allegro5/base.h>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>

#include "AudioHelper.hpp"
#include "WoodBullet.hpp"
#include "Group.hpp"
#include "PlugGunTurret.hpp"
#include "PlayScene.hpp"
#include "Point.hpp"
#include "ShootEffect.hpp"


const int PlugGunTurret::Price = 40;
PlugGunTurret::PlugGunTurret(float x, float y) :
    // TODO 3 (1/5): You can imitate the 2 files: 'PlugGunTurret.hpp', 'PlugGunTurret.cpp' to create a new turret.
    Turret("play/tower-base.png", "play/turret-6.png", x, y, 200, Price, 1.2) {
    // Move center downward, since we the turret head is slightly biased upward
    Anchor.y += 8.0f / GetBitmapHeight();
}
void PlugGunTurret::CreateBullet() {
    Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
    //std::cout << "This is diff.x: " << diff.x << " this is diff.y: " << diff.y << std::endl;
    float rotation = atan2(diff.y, diff.x);
    //std::cout << "This is rotation value: " << rotation << std::endl;
    Engine::Point normalized = diff.Normalize();
    // Change bullet position to the front of the gun barrel.
    getPlayScene()->BulletGroup->AddNewObject(new WoodBullet(Position + normalized * 36, diff, rotation, this));
    // TODO 4 (2/2): Add a ShootEffect here. Remember you need to include the class.
    getPlayScene()->EffectGroup->AddNewObject(new ShootEffect( this->Position + normalized * 36));
    AudioHelper::PlayAudio("gun.wav");
}
int PlugGunTurret::GetTurretID() {
    return 0;
}