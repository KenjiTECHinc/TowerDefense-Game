#ifndef ARMORPIERCEBULLET_HPP
#define ARMORPIERCEBULLET_HPP
#include "Bullet.hpp"

class Enemy;
class Turret;
namespace Engine {
    struct Point;
}  // namespace Engine

class ArmorPierceBullet : public Bullet {
public:
    explicit ArmorPierceBullet(Engine::Point position, Engine::Point forwardDirection, float rotation, Turret* parent);
    void OnExplode(Enemy* enemy) override;
};
#endif // WOODBULLET_HPP
#pragma once
