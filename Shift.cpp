#include <allegro5/base.h>
#include <cmath>
#include <string>
#include <vector>

#include "AudioHelper.hpp"
#include "Group.hpp"
#include "Shift.hpp"
#include "PlayScene.hpp"
#include "Point.hpp"


const int Shift::Price = 0;
Shift::Shift(float x, float y) :
    Turret("play/blank.png", "play/shift.png", x, y, 0, Price, 1.5) {
    Anchor.y += 8.0f / GetBitmapHeight();
}
int Shift::GetTurretID() {
    return 8;
}
void Shift::CreateBullet() {}
