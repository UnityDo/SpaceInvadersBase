#pragma once
#include <vector>
#include "Enemy.h"
#include "Bullet.h"

class EnemyManager {
public:
    EnemyManager();
    void Update(float dt);
    void Render(SDL_Renderer* renderer);
    void FireRandomBullet(std::vector<Bullet>& enemyBullets);
    std::vector<Enemy> enemies;
private:
    float direction = 1.0f; // 1 = derecha, -1 = izquierda
    float speed = 150.0f;   // Aumentado de 50 a 150
    float dropTimer = 0.0f;
    float moveTimer = 0.0f;
    float shootTimer = 0.0f;
};
