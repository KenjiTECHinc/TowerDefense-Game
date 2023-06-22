#include <allegro5/base.h>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <complex>
#include <iostream>
using namespace std;

#include "AudioHelper.hpp"
#include "OrbBullet.hpp"
#include "Group.hpp"
#include "Enemy.hpp"
#include "GameEngine.hpp"
#include "OrbTurret.hpp"
#include "PlayScene.hpp"
#include "Point.hpp"
#include "ShootEffect.hpp"

const int OrbTurret::Price = 100;
OrbTurret::OrbTurret(float x, float y , int bullet_cnt ) :
    // TODO 3 (1/5): You can imitate the 2 files: 'PlugGunTurret.hpp', 'PlugGunTurret.cpp' to create a new turret.
    Turret("play/tower-base.png", "play/turret-3.png", x, y, 100, Price, 3), Bullet_cnt(bullet_cnt) { //FIX ZONE: status = unresolved.
    // Move center downward, since we the turret head is slightly biased upward
    //Anchor.y += 8.0f / GetBitmapHeight();
	//Bullet_cnt = 0;

	this->Construct_flag = 1;
	cout << "Bullet count upon Orb construct: " << Bullet_cnt << endl;
	//this->BulletID = new map < int, OrbBullet*>(); //BulletID; //Fix Attempt
}
void OrbTurret::CreateBullet() {
	PlayScene* Scene = getPlayScene();
	if (!Scene) {
		return;
	}
	const float bulletRadius = 100.0f;
	const float bulletOrbitSpeed = 100.0f;
	float angle = bulletOrbitSpeed * getPlayScene()->ticks;
	Engine::Point bulletPosition = Position;

	//BUG ZONE: Initial issue = bullets amount doesn't match the previous one upon a shift.
	//BUG ZONE: status = resolved --> make cases and count helper.
	int creation_size, temp_count = 0;
	if (this->Bullet_cnt >= 0 && (this->Bullet_cnt <= 4) && this->Construct_flag == 1) creation_size = this->Bullet_cnt; //track how many bullets we currently have at construction time.
	else creation_size = 4; //else default at 4.
	for (int i = 0; i < creation_size; i++) { //create no more than the current amount we have. [MAX = 4]
		OrbBullet* bullet = new OrbBullet(bulletOrbitSpeed, 8, bulletPosition, Position, i * (ALLEGRO_PI / 2), this, bulletRadius, i);
		Scene->BulletGroup->AddNewObject(bullet);
		this->BulletID[i] = bullet; //add object to map. 
		temp_count++;
		//this->Bullet_cnt++;
	}
	cout << "The temp count is: " << temp_count << endl;
	if (temp_count != this->Bullet_cnt) this->Bullet_cnt = temp_count; //if the count in the end doesn't match.
	this->Construct_flag = 0; //Done construction of bullet.
}
int OrbTurret::GetTurretID() {
    return 3;
}

void OrbTurret::CheckBullet(bool inSight, float reload) {
	//cout << "This is the current bullet count: " << this->Bullet_cnt << endl;
	if ((this->Bullet_cnt >= 1) && this->Construct_flag == 0) {
		return;
	}
	else if (this->Bullet_cnt >= 0 && this->Construct_flag == 1) {
		CreateBullet();
	}
	else {
		CreateBullet();
	}
}

int OrbTurret::GetTurretBullet() {
	return this->Bullet_cnt;
}

void OrbTurret::DestructBullet() {
	//int temp_size = this->BulletID.size();
	for (int i = 0; i < 4; i++) {
		if(this->BulletID.find(i) != this->BulletID.end()) this->BulletID[i]->SelfDestruct();
	}
}
