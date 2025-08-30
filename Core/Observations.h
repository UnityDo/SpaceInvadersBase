#pragma once
#include <vector>

struct EnemyInfo { float x; float y; int hp; int type; };
struct PowerUpInfo { float x; float y; int type; };
struct BulletInfo { float x; float y; float vx; float vy; }; // enemy bullet information
struct WorldObservation {
    std::vector<EnemyInfo> enemies;
    std::vector<PowerUpInfo> powerups;
    std::vector<BulletInfo> enemyBullets; // new: enemy bullets for evasion
    float playerX = 0.0f;
    float playerY = 0.0f;
};
