#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include <cmath>
#include <random>
#include <string>
#include <vector>
#include <string>
#include <iostream>

//Spawnlings
#include "DiceCarrierEnemy2.hpp"
#include "DiceEnemy.hpp"

//extra resources
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

DiceCarrierEnemy2::DiceCarrierEnemy2(int x, int y) : Enemy("play/dice-3.png", x, y, 25, 40, 15, 15) {
	this->extraLives = 2;
	this->countDown = 6;
	this->hit_cnt = 0;
	//Dice Carrier Enemy. Spawn a Dice Enemy when die.
}

void DiceCarrierEnemy2::OnExplode() {
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

	//Enemy::getPlayScene()->enemyWaveData.emplace_front(2, 0); //need fix.
	//Enemy* enemy;
	//getPlayScene()->EnemyGroup->AddNewObject(enemy = new DiceEnemy(this->Position.x, this->Position.y));
	//std::cout << "Dice Enemy expression passed" << std::endl;
}

void DiceCarrierEnemy2::Hit(float damage) {
	hp -= damage;
	this->countDown--;
	if (hp <= 0) {
		OnExplode();
		// Remove all turret's reference to target.
		for (auto& it : lockedTurrets)
			it->Target = nullptr;
		for (auto& it : lockedBullets)
			it->Target = nullptr;
		getPlayScene()->EarnMoney(money);
		getPlayScene()->EnemyGroup->RemoveObject(objectIterator);
	
		//getPlayScene()->EnemyGroup->AddNewObject(new DiceCarrierEnemy(Position.x, Position.y));
		//getPlayScene()->EnemyGroup->RemoveObject(objectIterator);
		AudioHelper::PlayAudio("explosion.wav");
	}
	else if ((hp > 0) && (countDown <= 0)) {
		this->hit_cnt++;
		if (this->hit_cnt == 1) {
			getPlayScene()->EarnMoney(money); //earn money
			std::string newSpawn;
			if (dynamic_cast<Enemy*>(this)) newSpawn = "play/dice-3-2.png";
			dynamic_cast<Enemy*>(this)->Spawnling(newSpawn);
			this->speed = this->maxSpeed = 120;
			this->hp = 20;
			this->money = 20;
			this->CollisionRadius = 25;
			this->countDown = 3;
		}
		else if (this->hit_cnt == 2) {
			getPlayScene()->EarnMoney(money);
			std::string newSpawn;
			if (dynamic_cast<Enemy*>(this)) newSpawn = "play/dice-3-3.png";
			dynamic_cast<Enemy*>(this)->Spawnling(newSpawn);
			this->speed = this->maxSpeed = 160;
			this->hp = 25;
			this->money = 30;
			this->CollisionRadius = 25;
			this->countDown = INFINITY;
		}

	}
}
