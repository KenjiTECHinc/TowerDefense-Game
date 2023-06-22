#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include <cmath>
#include <random>
#include <string>
#include <vector>
#include <string>
#include <iostream>

//Spawnlings
#include "BossEnemy.hpp"
#include "TankEnemy.hpp"
#include "BlackNormalEnemy.hpp"
#include "BlueNormalEnemy.hpp"
#include "RedNormalEnemy.hpp"
//#include "DiceEnemy.hpp"

//extra resources
#include "ShootEffect.hpp"

#include "Turret.hpp"
#include "AudioHelper.hpp"
#include "Bullet.hpp"
#include "DirtyEffect.hpp"
#include "Enemy.hpp"
#include "ExplosionEffect.hpp"
#include "GameEngine.hpp"
#include "Group.hpp"
#include "IScene.hpp"
#include "LOG.hpp"
#include "PlayScene.hpp"

BossEnemy::BossEnemy(int x, int y) : Enemy("play/enemy-6.png", x, y, 40, 10, 300, 500) {
	this->extraLives = 2;
	//Dice Carrier Enemy. Spawn a Dice Enemy when die.
}

void BossEnemy::OnExplode() {
	//std::cout << "overriden for the dice carrier" << std::endl; //for debugging purposes
	getPlayScene()->EffectGroup->AddNewObject(new ExplosionEffect(Position.x, Position.y));
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> distId(1, 3);
	std::uniform_int_distribution<std::mt19937::result_type> dist(1, 20);
	for (int i = 0; i < 10; i++) {
		// Random add 10 dirty effects.
		getPlayScene()->GroundEffectGroup->AddNewObject(new DirtyEffect("play/dirty-" + std::to_string(distId(rng)) + ".png", dist(rng), Position.x, Position.y));
	}
	if (this->extraLives > 0) {
		Enemy* enemy;
		getPlayScene()->EnemyGroup->AddNewObject(enemy = new TankEnemy(Position.x, Position.y)); //spawn mini boss at spawn point
		enemy->UpdatePath(getPlayScene()->mapDistance);
		//getPlayScene()->EffectGroup->AddNewObject(new ShootEffect(enemy->Position * 36));
		//Enemy::getPlayScene()->enemyWaveData.emplace_front(6, 0); //if the boss die, but still have lives left then spawn enemy tank.
	}
	//Enemy* enemy;
	//getPlayScene()->EnemyGroup->AddNewObject(new DiceEnemy(this->Position.x, this->Position.y));
	//std::cout << "Dice Enemy expression passed" << std::endl;
}

void BossEnemy::Hit(float damage) {
	hp -= damage;
	if (hp <= 0) {
		OnExplode();
		// Remove all turret's reference to target.
		for (auto& it : lockedTurrets)
			it->Target = nullptr;
		for (auto& it : lockedBullets)
			it->Target = nullptr;
		getPlayScene()->EarnMoney(money);

		if (this->extraLives == 2) {
			this->extraLives--;
			this->speed = 30;
			this->hp = 200;
			this->money = 500;
		}
		else if (this->extraLives == 1) {
			this->extraLives--;
			this->speed = 60;
			this->hp = 100;
			this->money = 500;
		}
		else if (this->extraLives == 0) {
			getPlayScene()->EnemyGroup->RemoveObject(objectIterator);
		}
		//getPlayScene()->EnemyGroup->AddNewObject(new DiceCarrierEnemy(Position.x, Position.y));
		//getPlayScene()->EnemyGroup->RemoveObject(objectIterator);
		AudioHelper::PlayAudio("shockwave.ogg");
	}
	/*
	else if (hp > 0) {
		Enemy::getPlayScene()->enemyWaveData.emplace_front(7, 0); //if the boss still alive and got hit, then call reinforcement
	}*/
}

void BossEnemy::Update(float deltaTime) {
	//enemy spawns periodically.
	this->cooldown -= deltaTime;
	if (this->cooldown <= 0) {
		// shoot.
		this->cooldown = 8;
		Enemy* enemy;
		int ran_num = rand() % 3;
		if (ran_num == 2) getPlayScene()->EnemyGroup->AddNewObject(enemy = new BlackNormalEnemy(Position.x, Position.y));
		else if (ran_num == 1) getPlayScene()->EnemyGroup->AddNewObject(enemy = new BlueNormalEnemy(Position.x, Position.y));
		else getPlayScene()->EnemyGroup->AddNewObject(enemy = new BlackNormalEnemy(Position.x, Position.y));
		enemy->UpdatePath(getPlayScene()->mapDistance);

		/*
		for (int i = 0; i < 2; i++) { //make a wave spawn of new enemies.
			//Enemy* enemy;
			//getPlayScene()->EnemyGroup->AddNewObject(enemy = new BlackNormalEnemy(Position.x, Position.y)); //spawn mini boss at spawn point
			//enemy->UpdatePath(getPlayScene()->mapDistance);
			Enemy::getPlayScene()->enemyWaveData.emplace_front(7, 1); //random spawn points
		}*/
	}
	// Slow effect.
	SlowEffect(deltaTime);
	// Pre-calculate the velocity.
	float remainSpeed = speed * deltaTime;
	while (remainSpeed != 0) {
		if (path.empty()) {
			// Reach end point.
			Hit(hp);
			getPlayScene()->Hit();
			reachEndTime = 0;
			return;
		}
		Engine::Point target = path.back() * PlayScene::BlockSize + Engine::Point(PlayScene::BlockSize / 2, PlayScene::BlockSize / 2);
		Engine::Point vec = target - Position;
		// Add up the distances:
		// 1. to path.back()
		// 2. path.back() to border
		// 3. All intermediate block size
		// 4. to end point
		reachEndTime = (vec.Magnitude() + (path.size() - 1) * PlayScene::BlockSize - remainSpeed) / speed;
		Engine::Point normalized = vec.Normalize();
		if (remainSpeed - vec.Magnitude() > 0) {
			Position = target;
			path.pop_back();
			remainSpeed -= vec.Magnitude();
		}
		else {
			Velocity = normalized * remainSpeed / deltaTime;
			remainSpeed = 0;
		}
	}
	Rotation = atan2(Velocity.y, Velocity.x);
	Sprite::Update(deltaTime);
}