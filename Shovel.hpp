#ifndef SHOVEL_HPP
#define SHOVEL_HPP
#include "Turret.hpp"
#include <allegro5/bitmap.h>
#include <memory>
#include <vector>

#include "Sprite.hpp"

class Shovel : public Turret {
public:
    static const int Price;
    Shovel(float x, float y);
    int GetTurretID() override;
    void CreateBullet() override;
};
#endif // SHOVEL_HPP
