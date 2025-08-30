#pragma once
#include "Player.h"
#include "EnemyManager.h"
#include "Bullet.h"
#include "ParticleSystem.h"
#include "AudioManagerMiniaudio.h"
#include <vector>

// Forward declaration
class Game;

class CollisionManager {
public:
    CollisionManager(AudioManagerMiniaudio* audioManager);
    void CheckCollisions(Player& player, EnemyManager& enemies, std::vector<Bullet>& playerBullets, std::vector<Bullet>& enemyBullets, ParticleSystem& particles, Game& game, SDL_Renderer* renderer = nullptr);
    
private:
    AudioManagerMiniaudio* audioManager;
};
