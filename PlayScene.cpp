#include <allegro5/allegro.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <vector>
#include <queue>
#include <string>
#include <memory>
#include <iostream>
using namespace std;

#include "AudioHelper.hpp"
#include "DirtyEffect.hpp"
#include "Enemy.hpp"
#include "GameEngine.hpp"
#include "Group.hpp"
#include "IObject.hpp"
#include "Image.hpp"
#include "Label.hpp"
// Turret
#include "PlugGunTurret.hpp"
#include "MachineGunTurret.hpp"
#include "SniperTurret.hpp"
#include "OrbTurret.hpp"
#include "Plane.hpp"
#include "Shovel.hpp"
#include "Shift.hpp"

#include "DemoTurret.hpp"

// Enemy
#include "RedNormalEnemy.hpp"
#include "BlackNormalEnemy.hpp"
#include "DiceEnemy.hpp"
#include "DiceCarrierEnemy.hpp"
#include "DiceCarrierEnemy2.hpp"
#include "TankEnemy.hpp"

#include "BossEnemy.hpp"

#include "PlayScene.hpp"
#include "Resources.hpp"
#include "Sprite.hpp"
#include "Turret.hpp"
#include "TurretButton.hpp"
#include "LOG.hpp"

bool PlayScene::DebugMode = false;
const std::vector<Engine::Point> PlayScene::directions = { Engine::Point(-1, 0), Engine::Point(0, -1), Engine::Point(1, 0), Engine::Point(0, 1) };
const int PlayScene::MapWidth = 20, PlayScene::MapHeight = 13;
const int PlayScene::BlockSize = 64;
const float PlayScene::DangerTime = 7.61;
const Engine::Point PlayScene::SpawnGridPoint = Engine::Point(-1, 0);
const Engine::Point PlayScene::SpawnGridPoint2 = Engine::Point(MapWidth, 0); //new spawn point, top right corner.
const Engine::Point PlayScene::EndGridPoint = Engine::Point(MapWidth, MapHeight - 1);
// TODO 5 (2/3): Set the cheat code correctly.
const std::vector<int> PlayScene::code = { ALLEGRO_KEY_UP, ALLEGRO_KEY_DOWN, ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_ENTER };
Engine::Point PlayScene::GetClientSize() {
	return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
}
void PlayScene::Initialize() {
	// TODO 6 (1/2): There's a bug in this file, which crashes the game when you win. Try to find it.
	// TODO 6 (2/2): There's a bug in this file, which doesn't update the player's life correctly when getting the first attack. Try to find it.
	mapState.clear();
	keyStrokes.clear();
	ticks = 0;
	deathCountDown = -1;
	lives = 10;
	money = ((200 * MapId) - (50*MapId));
	SpeedMult = 1;
	// Add groups from bottom to top.
	AddNewObject(TileMapGroup = new Group());
	AddNewObject(GroundEffectGroup = new Group());
	AddNewObject(DebugIndicatorGroup = new Group());
	AddNewObject(TowerGroup = new Group());
	AddNewObject(EnemyGroup = new Group());
	AddNewObject(BulletGroup = new Group());
	AddNewObject(EffectGroup = new Group());
	// Should support buttons.
	AddNewControlObject(UIGroup = new Group());
	ReadMap();
	ReadEnemyWave();
	mapDistance = CalculateBFSDistance();
	ConstructUI();
	imgTarget = new Engine::Image("play/target.png", 0, 0);
	imgTarget->Visible = false;
	preview = nullptr;
	UIGroup->AddNewObject(imgTarget);
	// Preload Lose Scene
	deathBGMInstance = Engine::Resources::GetInstance().GetSampleInstance("astronomia.ogg");
	Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
	// Start BGM.
	// bgmId = AudioHelper::PlayBGM("play.ogg");
	if (!mute)
        bgmInstance = AudioHelper::PlaySample("play.ogg", true, AudioHelper::BGMVolume);
    else
        bgmInstance = AudioHelper::PlaySample("play.ogg", true, 0.0);
}
void PlayScene::Terminate() {
	AudioHelper::StopBGM(bgmId);
	AudioHelper::StopSample(bgmInstance);
	AudioHelper::StopSample(deathBGMInstance);
	deathBGMInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
	IScene::Terminate();
}
void PlayScene::Update(float deltaTime) {
	// If we use deltaTime directly, then we might have Bullet-through-paper problem.
	// Reference: Bullet-Through-Paper
	if (SpeedMult == 0)
		deathCountDown = -1;
	else if (deathCountDown != -1)
		SpeedMult = 1;
	// Calculate danger zone.
	std::vector<float> reachEndTimes;
	for (auto& it : EnemyGroup->GetObjects()) {
		reachEndTimes.push_back(dynamic_cast<Enemy*>(it)->reachEndTime);
	}
	// Can use Heap / Priority-Queue instead. But since we won't have too many enemies, sorting is fast enough.
	std::sort(reachEndTimes.begin(), reachEndTimes.end());
	float newDeathCountDown = -1;
	int danger = lives;
	for (auto& it : reachEndTimes) {
		if (it <= DangerTime) {
			danger--;
			if (danger <= 0) {
				// Death Countdown
				float pos = DangerTime - it;
				if (it > deathCountDown) {
					// Restart Death Count Down BGM.
					AudioHelper::StopSample(deathBGMInstance);
					if (SpeedMult != 0)
						deathBGMInstance = AudioHelper::PlaySample("astronomia.ogg", false, AudioHelper::BGMVolume, pos);
				}
				float alpha = pos / DangerTime;
				alpha = std::max(0, std::min(255, static_cast<int>(alpha * alpha * 255)));
				dangerIndicator->Tint = al_map_rgba(255, 255, 255, alpha);
				newDeathCountDown = it;
				break;
			}
		}
	}
	deathCountDown = newDeathCountDown;
	if (SpeedMult == 0)
		AudioHelper::StopSample(deathBGMInstance);
	if (deathCountDown == -1 && lives > 0) {
		AudioHelper::StopSample(deathBGMInstance);
		dangerIndicator->Tint.a = 0;
	}
	if (SpeedMult == 0)
		deathCountDown = -1;
	for (int i = 0; i < SpeedMult; i++) {
		IScene::Update(deltaTime);
		// Check if we should create new enemy.
		ticks += deltaTime;
		if (enemyWaveData.empty()) {
			//std::cout << "this is wave size: " << enemyWaveData.size() << std::endl;
			if (EnemyGroup->GetObjects().empty()) {
				// Free resources.
				/* TODO 6 a. (change to win scene).
				delete TileMapGroup;
				delete GroundEffectGroup;
				delete DebugIndicatorGroup;
				delete TowerGroup;
				delete EnemyGroup;
				delete BulletGroup;
				delete EffectGroup;
				delete UIGroup;
				delete imgTarget;
				*/
                Engine::GameEngine::GetInstance().ChangeScene("win"); //cahnge to win from win-scene
			}
			continue;
		}
		auto current = enemyWaveData.front();
		if (ticks < current.second)
			continue;
		ticks -= current.second;
		enemyWaveData.pop_front();
		const Engine::Point SpawnCoordinate = Engine::Point(SpawnGridPoint.x * BlockSize + BlockSize / 2, SpawnGridPoint.y * BlockSize + BlockSize / 2);
		Enemy* enemy;
		switch (current.first) {
		case 1:
			EnemyGroup->AddNewObject(enemy = new RedNormalEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
			break;
		case 2:
			EnemyGroup->AddNewObject(enemy = new DiceEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
			break;
		case 3:
			EnemyGroup->AddNewObject(enemy = new DiceCarrierEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
			break;
		case 4:
			EnemyGroup->AddNewObject(enemy = new DiceCarrierEnemy2(SpawnCoordinate.x, SpawnCoordinate.y));
			break;
		case 5:
			EnemyGroup->AddNewObject(enemy = new BossEnemy(SpawnCoordinate.x, SpawnCoordinate.y)); // Le~ Boss
			break;
		case 6:
			EnemyGroup->AddNewObject(enemy = new TankEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
			break;
		case 7:
			EnemyGroup->AddNewObject(enemy = new BlackNormalEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
			break;
		// TODO 2 (2/3): You need to modify 'resources/enemy1.txt', or 'resources/enemy2.txt' to spawn the new enemy.
		// The format is "[EnemyId] [TimeDelay] [Repeat]".
		// TODO 2 (3/3): Enable the creation of the new enemy.
		default:
			continue;
		}
		enemy->UpdatePath(mapDistance);
		// Compensate the time lost.
		enemy->Update(ticks);
	}
	if (preview) {
		preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
		// To keep responding when paused.
		preview->Update(deltaTime);
	}
}
void PlayScene::Draw() const {
	IScene::Draw();
	if (DebugMode) {
		// Draw reverse BFS distance on all reachable blocks.
		for (int i = 0; i < MapHeight; i++) {
			for (int j = 0; j < MapWidth; j++) {
				if (mapDistance[i][j] != -1) {
					// Not elegant nor efficient, but it's quite enough for debugging.
					Engine::Label label(std::to_string(mapDistance[i][j]), "pirulen.ttf", 32, (j + 0.5) * BlockSize, (i + 0.5) * BlockSize);
					label.Anchor = Engine::Point(0.5, 0.5);
					label.Draw();
				}
			}
		}
	}
}
void PlayScene::OnMouseDown(int button, int mx, int my) {
	if ((button & 1) && !imgTarget->Visible && preview) {
		// Cancel turret construct.
		UIGroup->RemoveObject(preview->GetObjectIterator());
		preview = nullptr;
	}
	IScene::OnMouseDown(button, mx, my);
}
void PlayScene::OnMouseMove(int mx, int my) {
	IScene::OnMouseMove(mx, my);
	const int x = mx / BlockSize;
	const int y = my / BlockSize;
	if (!preview || x < 0 || x >= MapWidth || y < 0 || y >= MapHeight) {
		imgTarget->Visible = false;
		return;
	}
	imgTarget->Visible = true;
	imgTarget->Position.x = x * BlockSize;
	imgTarget->Position.y = y * BlockSize;
}
void PlayScene::OnMouseUp(int button, int mx, int my) {
	IScene::OnMouseUp(button, mx, my);
	if (!imgTarget->Visible)
		return;
	const int x = mx / BlockSize;
	const int y = my / BlockSize;
	if (button & 1) {
		if ((mapState[y][x] != TILE_OCCUPIED) && (mapState[y][x] !=TILE_MACHINEGUN) && (mapState[y][x] !=TILE_MAXED)) {
			//cout << "entered this !Tile case" << endl;
			if (!preview)
				return;
			// Check if valid. 
			if (preview->GetTurretID() != 7 && preview->GetTurretID()!=8) {
				if (!CheckSpaceValid(x, y)) { //Original is !checkspacevalid(x,y) || preview get turret id == 7)...
					std::cout << "checked invalid 1" << std::endl;
					Engine::Sprite* sprite;
					GroundEffectGroup->AddNewObject(sprite = new DirtyEffect("play/target-invalid.png", 1, x * BlockSize + BlockSize / 2, y * BlockSize + BlockSize / 2));
					sprite->Rotation = 0;
					return;
				}
			}
			else {
				//BUG ZONE: Initial issue = shovel & shifter became valid on empty tiles.
				//BUG ZONE: Status = resolved --> add conditions on which tile_type it can be placed on. 
				if (((preview->GetTurretID() == 7)||(preview->GetTurretID()==8)) && (mapState[y][x] == TILE_FLOOR || mapState[y][x] == TILE_REACTOR || mapState[y][x] == TILE_DIRT)) {
					std::cout << "checked invalid 2" << std::endl;
					Engine::Sprite* sprite;
					GroundEffectGroup->AddNewObject(sprite = new DirtyEffect("play/target-invalid.png", 1, x * BlockSize + BlockSize / 2, y * BlockSize + BlockSize / 2));
					sprite->Rotation = 0;
					return;
				}
			}
			// Purchase.
			EarnMoney(-preview->GetPrice(1));
			preview->price = preview->GetPrice(2); //Debug pricing
			std::cout << "purchased " << preview->GetTurretID() << std::endl;
			// Remove Preview.
			preview->GetObjectIterator()->first = false;
			UIGroup->RemoveObject(preview->GetObjectIterator());
			// Construct real turret.
			preview->Position.x = x * BlockSize + BlockSize / 2;
			preview->Position.y = y * BlockSize + BlockSize / 2;
			preview->Enabled = true;
			preview->Preview = false;
			preview->Tint = al_map_rgba(255, 255, 255, 255);

			TowerGroup->AddNewObject(preview);
			//BUG ZONE: Initial issue = machine gun can be placed on top of other turrets.
			//BUG ZONE: status = resolved --> added a new tile to identify a lvl 1 machine gun and a max level machine gun.
			if (preview->GetTurretID() == 1) {
				cout << "this is y and x value: " << y << ' ' << x << endl;
				mapState[y][x] = TILE_MACHINEGUN;
				cout << "made map state machine gun" << endl;
			}
			if (preview->GetTurretID() != 1) {
				mapState[y][x] = TILE_OCCUPIED;
				cout << "made map state normally occupied" << endl;
			}

			// To keep responding when paused.
			preview->Update(0);
			// Remove Preview.
			preview = nullptr;
			OnMouseMove(mx, my);
		}
		else {
			if (!preview)
				return;
			// Check if valid.
			if (((!CheckSpaceValid(x, y)) || (mapState[y][x] == TILE_OCCUPIED) || (mapState[y][x] == TILE_MAXED) || (mapState[y][x] == TILE_MACHINEGUN)) && ((preview->GetTurretID()!= 1) && (preview->GetTurretID()!=7) && (preview->GetTurretID()!=8))) {
				std::cout << "this is invalid: " << preview->GetTurretID() << std::endl;
				Engine::Sprite* sprite;
				GroundEffectGroup->AddNewObject(sprite = new DirtyEffect("play/target-invalid.png", 1, x * BlockSize + BlockSize / 2, y * BlockSize + BlockSize / 2));
				sprite->Rotation = 0;
				return;
			}
			if (preview->GetTurretID() == 1) {
				cout << "turret machine gun ID matched, proceed to check tiles" << endl;
				if ((mapState[y][x] == TILE_MAXED) || (mapState[y][x] != TILE_MACHINEGUN)){
					cout << "map state maxed evoked." << endl;
					cout << "this is y and x: " << y << ' ' << x << endl;
					//cout << mapState[y][x] << endl;
					Engine::Sprite* sprite;
					GroundEffectGroup->AddNewObject(sprite = new DirtyEffect("play/target-invalid.png", 1, x * BlockSize + BlockSize / 2, y * BlockSize + BlockSize / 2));
					sprite->Rotation = 0;
					return;
				}
			}
			
			// Remove Preview.
			preview->GetObjectIterator()->first = false;
			UIGroup->RemoveObject(preview->GetObjectIterator());
			// Construct real turret.
			preview->Position.x = x * BlockSize + BlockSize / 2;
			preview->Position.y = y * BlockSize + BlockSize / 2;
			preview->Enabled = true;
			preview->Preview = false;
			preview->Tint = al_map_rgba(255, 255, 255, 255);

			
			auto towers = TowerGroup->GetObjects();
			Engine::Point pnt;
			towers = TowerGroup->GetObjects();
			pnt.x = preview->Position.x;
			pnt.y = preview->Position.y;
			for (auto& it : towers) {
				IObject* tower = dynamic_cast<IObject*>(it);
				if (tower->Position.x == pnt.x && tower->Position.y == pnt.y) {
					if (preview->GetTurretID() == 1) {
						if ((preview->GetTurretID() == 1 && dynamic_cast<Turret*>(tower)->GetTurretID() == 1) && (dynamic_cast<Turret*>(tower)->lvl < 2)) {
							
							std::string upgradeTurret;
							if (dynamic_cast<Turret*>(tower)->GetTurretID() == 1) upgradeTurret = "play/turret-2.png";
							dynamic_cast<Turret*>(tower)->UpTurret(upgradeTurret);
							dynamic_cast<Turret*>(tower)->lvl++;
							EarnMoney(-(preview->GetPrice(1)));
							//preview->price = preview->GetPrice(2); //Unnecessary??
							dynamic_cast<Turret*>(tower)->price = 120;
							//BUG ZONE: Initial issue = machine gun turret placable on other turrets.
							mapState[y][x] = TILE_MAXED; //change tile to max level marker.
							preview->Update(0);
							// Remove Preview.
							preview = nullptr;
							OnMouseMove(mx, my);
							return;
						}
					}
					else if (preview->GetTurretID() == 7) {
						
						EarnMoney(dynamic_cast<Turret*>(tower)->GetPrice(1) / 2);
						if (dynamic_cast<Turret*>(tower)->GetTurretID() == 3) { //if it has external bullet, go to destruct function.
							dynamic_cast<OrbTurret*>(tower)->DestructBullet();
						}
						//std::cout << "The fawking price: " << dynamic_cast<Turret*>(tower)->GetPrice(1) << std::endl; //Debug pricing
						TowerGroup->RemoveObject((tower)->GetObjectIterator());
						mapState[y][x] = TILE_FLOOR;
						preview->Update(0);
						preview = nullptr;
						OnMouseMove(mx, my);
						return;
					}
					else if (preview->GetTurretID() == 8) {

						

						if (dynamic_cast<Turret*>(tower)->GetTurretID() == 0)
							preview = new PlugGunTurret(0, 0);
						else if (dynamic_cast<Turret*>(tower)->GetTurretID() == 1) { //move machine gun.
							preview = new MachineGunTurret(0, 0);
							if (dynamic_cast<Turret*>(tower)->lvl == 2) { //check machine gun level.
								dynamic_cast<Turret*>(preview)->lvl += 1;
								dynamic_cast<Turret*>(preview)->UpTurret("play/turret-2.png");
							}
						}
						else if (dynamic_cast<Turret*>(tower)->GetTurretID() == 2)
							preview = new SniperTurret(0, 0);

						//BUG ZONE: Initial issue = Turret's bullet doesn't change when shifted. Turret's bullet remain moving when shifted. 
						//BUG ZONE: status = resolved --> get the initial bullet count.
						else if (dynamic_cast<Turret*>(tower)->GetTurretID() == 3) {
							int temp_bullet_cnt = dynamic_cast<Turret*>(tower)->GetTurretBullet();
							//cout << "The current bullet count: " << temp_bullet_cnt << endl; //debug
							preview = new OrbTurret(0, 0, temp_bullet_cnt); //FIX ZONE: Orb Turret >> status = unresolved.
						}
						else if (dynamic_cast<Turret*>(tower)->GetTurretID() == 10) {
							preview = new DemoTurret(0, 0);
						}
						//BUG ZONE: Initial issue = Orb turret doesn't remove the bullet upon delete.
						//BUG ZONE: status = resolved --> add public function for removing created objects.
						//BUG ZONE: Idea = either remove the bullet objects here or inside the orb turret's class. ^^

						//BUG ZONE: Initial issue = Turret doesn't refund --> price set to 0.
						//BUG ZONE: status = resolved --> added protected and unprotected attribute.

						dynamic_cast<Turret*>(preview)->price = 0;
						if (dynamic_cast<Turret*>(tower)->GetTurretID() == 3) {
							dynamic_cast<OrbTurret*>(tower)->DestructBullet();
						}
						TowerGroup->RemoveObject(tower->GetObjectIterator());
						mapState[y][x] = TILE_FLOOR;
						preview->Tint = al_map_rgba(255, 255, 255, 200);
						preview->Enabled = false;
						preview->Preview = true;
						UIGroup->AddNewObject(preview);
						OnMouseMove(mx, my);
						return;
					}
				}
			}
			//TowerGroup->AddNewObject(preview);
			// To keep responding when paused.
			cout << "machine gun must be created here." << endl;
			preview->Update(0);
			// Remove Preview.
			preview = nullptr;
			mapState[y][x] = TILE_OCCUPIED;
			OnMouseMove(mx, my);

		}
	}
}
void PlayScene::OnKeyDown(int keyCode) {
	IScene::OnKeyDown(keyCode);
	if (keyCode == ALLEGRO_KEY_TAB) {
		// TODO 5 (1/3): Set Tab as a code to active / de-active the debug mode.
		this->DebugMode = !DebugMode;
	}
	else {
		keyStrokes.push_back(keyCode);
		if (keyStrokes.size() > code.size())
			keyStrokes.pop_front();
		std::vector<int>::const_iterator ptr = code.begin();
		int flag = 1;
		for (auto user_ptr = keyStrokes.begin(); user_ptr != keyStrokes.end(); user_ptr++, ptr++) {
			if (*user_ptr != *ptr) {
				flag = 0;
				break;
			}
		}
		if (flag == 1 && keyStrokes.size()==code.size()) {
			// TODO 5 (3/3): Check whether the input sequence corresponds to the code. If so, active a plane and earn 10000 money.
			// Active a plane : 
			EffectGroup->AddNewObject(new Plane());
			// Earn money : 
			this->money += 10000;
		}
	
}
	if (keyCode == ALLEGRO_KEY_Q) {
		// Hotkey for PlugGunTurret.
		UIBtnClicked(0);
	}
	// TODO 3 (5/5): Make the W key to create the new turret.
	else if (keyCode == ALLEGRO_KEY_W) {
		UIBtnClicked(1);
		// Hotkey for new turret.
	}
	else if (keyCode == ALLEGRO_KEY_E) {
		UIBtnClicked(2);
	}
	else if (keyCode == ALLEGRO_KEY_R) {
		UIBtnClicked(3);
	}
	else if (keyCode == ALLEGRO_KEY_S) {
		if((!preview) || (preview->GetTurretID()!=7)) UIBtnClicked(7);
	}
	else if (keyCode == ALLEGRO_KEY_A) {
		if(!preview) UIBtnClicked(8);
	}
	else if (keyCode == ALLEGRO_KEY_P) { //insta win scene
		Engine::GameEngine::GetInstance().ChangeScene("win");
	}
	/*
	else if (keyCode == ALLEGRO_KEY_C) { //cancel button - temporary
		UIBtnClicked(15);
	}*/
	else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9) {
		// Hotkey for Speed up.
		SpeedMult = keyCode - ALLEGRO_KEY_0;
	}
	else if (keyCode == ALLEGRO_KEY_M) {
		// Hotkey for mute / unmute.
        if (mute)
            AudioHelper::ChangeSampleVolume(bgmInstance, AudioHelper::BGMVolume);
        else
            AudioHelper::ChangeSampleVolume(bgmInstance, 0.0);
        mute = !mute;
	}
}
void PlayScene::Hit() {
	UILives->Text = std::string("Life ") + std::to_string(--lives); //TODO 6 (b.) change to pre-decrement instead >.<
	if (lives <= 0) {
		//std::cout << "this lose scene occured" << std::endl;
		Engine::GameEngine::GetInstance().ChangeScene("lose"); //new way to change scene..??
		
	}
}
int PlayScene::GetMoney() const {
	return money;
}
void PlayScene::EarnMoney(int money) {
	this->money += money;
	UIMoney->Text = std::string("$") + std::to_string(this->money);
}
void PlayScene::ReadMap() {
	//std::cout << "map id: " << MapId << std::endl;
	std::string filename = std::string("resources/map") + std::to_string(MapId) + ".txt";
	// Read map file.
	char c;
	std::vector<int> mapData; //change vector type to <int>
	std::ifstream fin(filename);
	while (fin >> c) {
		switch (c) {
		case '0': mapData.push_back(0); break; //push back int value accordingly.
		case '1': mapData.push_back(1); break;
		case '2': mapData.push_back(2); break;
		case '\n':
		case '\r':
			if (static_cast<int>(mapData.size()) / MapWidth != 0)
				throw std::ios_base::failure("Map data is corrupted.");
			break;
		default: throw std::ios_base::failure("Map data is corrupted.");
		}
	}
	fin.close();
	// Validate map data.
	if (static_cast<int>(mapData.size()) != MapWidth * MapHeight)
		throw std::ios_base::failure("Map data is corrupted.");
	// Store map in 2d array.
	mapState = std::vector<std::vector<TileType>>(MapHeight, std::vector<TileType>(MapWidth));
	for (int i = 0; i < MapHeight; i++) {
		for (int j = 0; j < MapWidth; j++) {
			const int num = mapData[i * MapWidth + j];
			if (num == 1) mapState[i][j] = TILE_FLOOR;
			else if (num == 2) mapState[i][j] = TILE_REACTOR; //add new Tile in playscene.hpp
			else mapState[i][j] = TILE_DIRT;
			//mapState[i][j] = num? TILE_FLOOR : TILE_DIRT;
			if (num) { //for adding new Tiles
				std::cout << "map number: " << num << std::endl;
				if (num == 1) {
					TileMapGroup->AddNewObject(new Engine::Image("play/floorHD.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
				}
				else if (num == 2) { //if-else case to read map tile type.
					TileMapGroup->AddNewObject(new Engine::Image("play/react_floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
				}
			}
			else
				TileMapGroup->AddNewObject(new Engine::Image("play/dirtHD.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
		}
	}
}
void PlayScene::ReadEnemyWave() {
	std::string filename = std::string("resources/enemy") + std::to_string(MapId) + ".txt";
	// Read enemy file.
	float type, wait, repeat;
	enemyWaveData.clear();
	std::ifstream fin(filename);
	while (fin >> type && fin >> wait && fin >> repeat) {
		for (int i = 0; i < repeat; i++)
			enemyWaveData.emplace_back(type, wait);
	}
	fin.close();
}
void PlayScene::ConstructUI() {
	// Background
	UIGroup->AddNewObject(new Engine::Image("play/sand.png", 1280, 0, 320, 832));
	// Text
	UIGroup->AddNewObject(new Engine::Label(std::string("Stage ") + std::to_string(MapId), "pirulen.ttf", 32, 1294, 0));
	UIGroup->AddNewObject(UIMoney = new Engine::Label(std::string("$") + std::to_string(money), "pirulen.ttf", 24, 1294, 48));
	UIGroup->AddNewObject(UILives = new Engine::Label(std::string("Life ") + std::to_string(lives), "pirulen.ttf", 24, 1294, 88));
	// Buttons
	ConstructButton(0, 1,0, "play/turret-6.png", PlugGunTurret::Price);
	ConstructButton(1, 1,1, "play/turret-1.png", MachineGunTurret::Price);
	ConstructButton(2, 1,2, "play/turret-11.png", SniperTurret::Price);
	ConstructButton(3, 1, 3, "play/turret-3.png", OrbTurret::Price);
	ConstructButton(10, 2, 0, "play/turret-7.png", DemoTurret::Price);

	//items buttons
	ConstructButton(7, 3, 0, "play/shovel.png", Shovel::Price);
	ConstructButton(8, 3, 1, "play/shift.png", Shift::Price);
	//ConstructButton(15, 5,0, "play/turret-2.png", 0); //cancel button
	// TODO 3 (3/5): Create a button to support constructing the new turret.
    
	int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
	int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
	int shift = 135 + 25;
	dangerIndicator = new Engine::Sprite("play/benjamin.png", w - shift, h - shift);
	dangerIndicator->Tint.a = 0;
	UIGroup->AddNewObject(dangerIndicator);
}

void PlayScene::ConstructButton(int id, int row, int col, std::string sprite, int price) {
	TurretButton* btn;
	if (id != 7 && id != 8 && id!=3) {
		btn = new TurretButton("play/floor.png", "play/dirt.png",
			Engine::Sprite("play/tower-base.png", 1294 + col * 76, 136 * row, 0, 0, 0, 0),
			Engine::Sprite(sprite, 1294 + col * 76, (136 * row) - 8, 0, 0, 0, 0) //<-- move the comma here instead. 
			, 1294 + col * 76, 136 * row, price);
	}
	else {
		btn = new TurretButton("play/floor.png", "play/dirt.png",
			Engine::Sprite("play/blank.png", 1294 + col * 76, 136 * row, 0, 0, 0, 0),
			Engine::Sprite(sprite, 1294 + col * 76, (136 * row), 0, 0, 0, 0) //<-- move the comma here instead. 
			, 1294 + col * 76, 136 * row, price);
	}
	// Reference: Class Member Function Pointer and std::bind.
	btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, id));
	UIGroup->AddNewControlObject(btn);
}

void PlayScene::UIBtnClicked(int id) {
	if (preview) {
		UIGroup->RemoveObject(preview->GetObjectIterator());
		preview = nullptr;
	}
	if (id == 0 && money >= PlugGunTurret::Price)
		preview = new PlugGunTurret(0, 0);

	if (id == 1 && money >= MachineGunTurret::Price)
		preview = new MachineGunTurret(0, 0);
	if (id == 2 && money >= SniperTurret::Price)
		preview = new SniperTurret(0, 0);
	if (id == 3 && money >= OrbTurret::Price)
		preview = new OrbTurret(0, 0, 0); //FIX ZONE: >> OrbTurret >> status = unresolved.
	if (id == 7 && money >= Shovel::Price)
		preview = new Shovel(0, 0);
	if (id == 8 && money >= Shift::Price)
		preview = new Shift(0, 0);
	if (id == 10 && money >= DemoTurret::Price)
		preview = new DemoTurret(0, 0);
	/*
	if (id == 15 && money >= 0) { // point to NULL
		preview = nullptr;
	}*/
	// TODO 3 (4/5): On the new turret button callback, create the new turret.

	if (!preview)
		return;
	preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
	preview->Tint = al_map_rgba(255, 255, 255, 200);
	preview->Enabled = false;
	preview->Preview = true;
	UIGroup->AddNewObject(preview);
	OnMouseMove(Engine::GameEngine::GetInstance().GetMousePosition().x, Engine::GameEngine::GetInstance().GetMousePosition().y);

}

//BUG ZONE: Initial issue  = machine gun placeable on regular turret's tiles and upgraded machine gun.
//BUG ZONE: status = resolved --> added new tiles.
bool PlayScene::CheckSpaceValid(int x, int y) {
	if (x < 0 || x >= MapWidth || y < 0 || y >= MapHeight || mapState[y][x] == TILE_DIRT) //added
		return false;

	auto map00 = mapState[y][x];
	if (mapState[y][x] == TILE_MACHINEGUN) mapState[y][x] = TILE_MACHINEGUN;
	else mapState[y][x] = TILE_OCCUPIED;
	std::vector<std::vector<int>> map = CalculateBFSDistance();
	mapState[y][x] = map00;
	if (map[0][0] == -1)
		return false;
	for (auto& it : EnemyGroup->GetObjects()) {
		Engine::Point pnt;
		pnt.x = floor(it->Position.x / BlockSize);
		pnt.y = floor(it->Position.y / BlockSize);
		if (pnt.x < 0) pnt.x = 0;
		if (pnt.x >= MapWidth) pnt.x = MapWidth - 1;
		if (pnt.y < 0) pnt.y = 0;
		if (pnt.y >= MapHeight) pnt.y = MapHeight - 1;
		if (map[pnt.y][pnt.x] == -1)
			return false;
	}
	// All enemy have path to exit.
	//mapState[y][x] = TILE_OCCUPIED;
	if (mapState[y][x] == TILE_MACHINEGUN) mapState[y][x] = TILE_MACHINEGUN;
	else mapState[y][x] = TILE_OCCUPIED;
	mapDistance = map;
	for (auto& it : EnemyGroup->GetObjects())
		dynamic_cast<Enemy*>(it)->UpdatePath(mapDistance);
	return true;
}
std::vector<std::vector<int>> PlayScene::CalculateBFSDistance() {
	// Reverse BFS to find path.
	std::vector<std::vector<int>> map(MapHeight, std::vector<int>(std::vector<int>(MapWidth, -1)));
	std::queue<Engine::Point> que;
	// Push end point.
	// BFS from end point.
	if (mapState[MapHeight - 1][MapWidth - 1] != TILE_DIRT)
		return map;
	que.push(Engine::Point(MapWidth - 1, MapHeight - 1));
	map[MapHeight - 1][MapWidth - 1] = 0;
	while (!que.empty()) {
		Engine::Point p = que.front();
		que.pop();
        for (auto &c : directions) {
            int x = p.x + c.x;
            int y = p.y + c.y;
            if (x < 0 || x >= MapWidth || y < 0 || y >= MapHeight ||
                map[y][x] != -1 || mapState[y][x] != TILE_DIRT) {
                continue;
            } else {
                map[y][x] = map[p.y][p.x] + 1;
                que.push(Engine::Point(x, y));
            }
        }
	}
	return map;
}
