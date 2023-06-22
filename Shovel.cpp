#include <allegro5/base.h>
#include <cmath>
#include <string>
#include <vector>

#include "AudioHelper.hpp"
#include "Group.hpp"
#include "Shovel.hpp"
#include "PlayScene.hpp"
#include "Point.hpp"


const int Shovel::Price = 0;
Shovel::Shovel(float x, float y) :
    Turret("play/blank.png", "play/shovel.png", x, y, 0, Price, 1.5) {
    Anchor.y += 8.0f / GetBitmapHeight();
}
int Shovel::GetTurretID() {
    return 7;
}
void Shovel::CreateBullet() {}
