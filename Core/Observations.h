#pragma once
#include <vector>

struct EnemyInfo { float x; float y; int hp; int type; };
struct PowerUpInfo { float x; float y; int type; };
struct WorldObservation {
    std::vector<EnemyInfo> enemies;
    std::vector<PowerUpInfo> powerups;
    float playerX = 0.0f;
    float playerY = 0.0f;
};
