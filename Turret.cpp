#include <allegro5/color.h>
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include <utility>
#include <iostream>

#include "Enemy.hpp"
#include "GameEngine.hpp"
#include "Group.hpp"
#include "IObject.hpp"
#include "IScene.hpp"
#include "PlayScene.hpp"
#include "Point.hpp"
#include "Turret.hpp"

PlayScene* Turret::getPlayScene() {
	return dynamic_cast<PlayScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
}
Turret::Turret(std::string imgBase, std::string imgTurret, float x, float y, float radius, int price, float coolDown) :
	Sprite(imgTurret, x, y), price(price), coolDown(coolDown), imgBase(imgBase, x, y), const_price(price){
	CollisionRadius = radius;
}

//BUG ZONE: Initial issue = Orbital turret move when enemy is within vicinity.
//BUG ZONE: status = resolved --> do no allow orbit turret to be able to lock-on a target.
void Turret::Update(float deltaTime) {
	Sprite::Update(deltaTime);
	PlayScene* scene = getPlayScene();
	imgBase.Position = Position;
	imgBase.Tint = Tint;
	if (!Enabled)
		return;
	if (Target && this->GetTurretID()!=3) {
		Engine::Point diff = Target->Position - Position;
		if (diff.Magnitude() > CollisionRadius) {
			Target->lockedTurrets.erase(lockedTurretIterator);
			Target = nullptr;
			lockedTurretIterator = std::list<Turret*>::iterator();
		}
	}
	if (!Target && this->GetTurretID()!=3) {
		// Lock first seen target.
		// Can be improved by Spatial Hash, Quad Tree, ...
		// However simply loop through all enemies is enough for this program.
		for (auto& it : scene->EnemyGroup->GetObjects()) {
			Engine::Point diff = it->Position - Position;
			if (diff.Magnitude() <= CollisionRadius) {
				Target = dynamic_cast<Enemy*>(it);
				Target->lockedTurrets.push_back(this);
				lockedTurretIterator = std::prev(Target->lockedTurrets.end());
				break;
			}
		}
	}
	if (Target && this->GetTurretID() != 3) {
			Engine::Point originRotation = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
			Engine::Point targetRotation = (Target->Position - Position).Normalize();
			float maxRotateRadian = 4 * rotateRadian * deltaTime; //fix rotation speed to minimize the pre-firing issue.
			float cosTheta = originRotation.Dot(targetRotation);
			// Might have floating-point precision error.
			if (cosTheta > 1) cosTheta = 1;
			else if (cosTheta < -1) cosTheta = -1;
			float radian = acos(cosTheta);
			Engine::Point rotation;
			if (abs(radian) <= maxRotateRadian)
				rotation = targetRotation;
			else
				rotation = ((abs(radian) - maxRotateRadian) * originRotation + maxRotateRadian * targetRotation) / radian;
			// Add 90 degrees (PI/2 radian), since we assume the image is oriented upward.
			Rotation = atan2(rotation.y, rotation.x) + ALLEGRO_PI / 2;
			// Shoot reload.
	}
	bool inSight = (Target) ? true : false;
	reload -= deltaTime;
	CheckBullet(inSight, reload); //call function to check the bullet count.
	if (reload <= 0) {
			// shoot.
			reload = coolDown;
			//CreateBullet(); moved inside check bullet instead
			//reload += 0.05; //provide break interval point after shooting
	
	}
}
void Turret::Draw() const {
	if (Preview) {
		al_draw_filled_circle(Position.x, Position.y, CollisionRadius, al_map_rgba(0, 255, 0, 50));
	}
	imgBase.Draw();
	Sprite::Draw();
	if (PlayScene::DebugMode) {
		// Draw target radius.
		al_draw_circle(Position.x, Position.y, CollisionRadius, al_map_rgb(0, 0, 255), 2);
	}
}

void Turret::CheckBullet(bool inSight, float reload) {
	if (inSight == false) {
		return;
	}
	if (reload <= 0) {
		CreateBullet();
	}
}

void Turret::UpTurret(std::string imgNewTurret) {
	Engine::Sprite upgradeTurret(imgNewTurret, Position.x, Position.y);
	std::swap(Sprite::bmp, upgradeTurret.bmp);
}

int Turret::GetPrice(int request) const {
	if (request == 1) return price;
	else if (request == 2) return const_price;
}

int Turret::GetTurretID(){
	return -1;
}

int Turret::GetTurretBullet() {
	return Bullet_cnt;
}
