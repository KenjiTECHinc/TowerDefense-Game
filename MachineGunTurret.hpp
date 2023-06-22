#ifndef MACHINEGUNTURRET_HPP
#define MACHINEGUNTURRET_HPP
#include "Turret.hpp"
#include <allegro5/bitmap.h>
#include <memory>
#include <vector>

#include "Sprite.hpp"

class MachineGunTurret : public Turret {
public:
    //Sprite imgLVL2;
    static const int Price;
    MachineGunTurret(float x, float y);
    void CreateBullet() override;
    int GetTurretID() override;
    void UpTurret(std::string newImgTurret) override;
};
#endif 