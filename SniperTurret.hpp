#ifndef SNIPERTURRET_HPP
#define SNIPERTURRET_HPP
#include "Turret.hpp"
#include <allegro5/bitmap.h>
#include <memory>
#include <vector>

#include "Sprite.hpp"

class SniperTurret : public Turret {
public:
    static const int Price;
    SniperTurret(float x, float y);
    void CreateBullet() override;
    int GetTurretID() override;
};
#endif 