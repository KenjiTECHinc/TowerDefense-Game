#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include <cmath>
#include <random>
#include <string>
#include <vector>
#include <string>
#include <iostream>

//Spawnlings
#include "DiceCarrierEnemy.hpp"
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

DiceCarrierEnemy::DiceCarrierEnemy(int x, int y) : Enemy("play/dice-2.png", x, y, 22, 60, 8, 10) {
	//Dice Carrier Enemy. Spawn a Dice Enemy when die.
}

void DiceCarrierEnemy::OnExplode() {
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

void DiceCarrierEnemy::Hit(float damage) {
	hp -= damage;
	if (hp <= 0) {
		OnExplode();
		// Remove all turret's reference to target.
		for (auto& it : lockedTurrets) {
			it->Target = nullptr;
		}
		for (auto& it : lockedBullets)
			it->Target = nullptr;
		getPlayScene()->EarnMoney(money);
		//this->Visible = false;
		
		/*if (this->extraLives == 1) {
			this->extraLives--;
			std::string newSpawn;
			if (dynamic_cast<Enemy*>(this)) newSpawn = "play/dice-1.png";
			dynamic_cast<Enemy*>(this)->Spawnling(newSpawn);
			this->speed = 80;
			this->hp = 5;
			this->money = 7;
			this->CollisionRadius = 25;
			//this->Visible = true;
		}
		else if (this->extraLives == 0) {
			getPlayScene()->EnemyGroup->RemoveObject(objectIterator);
		}*/
		Enemy* enemy;
		getPlayScene()->EnemyGroup->AddNewObject(enemy = new DiceEnemy(Position.x, Position.y));
		enemy->UpdatePath(getPlayScene()->mapDistance);
		getPlayScene()->EnemyGroup->RemoveObject(objectIterator);
		AudioHelper::PlayAudio("explosion.wav");
	}
}

