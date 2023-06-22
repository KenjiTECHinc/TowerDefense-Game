#ifndef SHIFT_HPP
#define SHIFT_HPP
#include "Turret.hpp"
#include <allegro5/bitmap.h>
#include <memory>
#include <vector>

#include "Sprite.hpp"

class Shift : public Turret {
public:
    static const int Price;
    Shift(float x, float y);
    int GetTurretID() override;
    void CreateBullet() override;
};
#endif // SHIFT_HPP
