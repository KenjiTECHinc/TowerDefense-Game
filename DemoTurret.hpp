#ifndef DEMOTURRET_HPP
#define DEMOTURRET_HPP
#include "Turret.hpp"
#include <allegro5/bitmap.h>
#include <memory>
#include <vector>

#include "Sprite.hpp"

class DemoTurret : public Turret {
public:
    static const int Price;
    DemoTurret(float x, float y);
    void CreateBullet() override;
    int GetTurretID() override;
};
#endif // DEMOTURRET_HPP
