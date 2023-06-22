#ifndef SHOOTEFFECTBURST_HPP
#define SHOOTEFFECTBURST_HPP
#include <allegro5/bitmap.h>
#include <memory>
#include <vector>

#include "Sprite.hpp"
//Copied from the ExplosionEffect header file.
class PlayScene;

class ShootEffectBurst : public Engine::Sprite {
protected:
	PlayScene* getPlayScene();
	float timeTicks;
	std::vector<std::shared_ptr<ALLEGRO_BITMAP>> bmps;
	float timeSpan = 0.5;
public:
	ShootEffectBurst(Engine::Point position/*, Engine::Point y*/); //use Engine::Point instead to extract singular XY position.
	void Update(float deltaTime) override;
};
#endif 
