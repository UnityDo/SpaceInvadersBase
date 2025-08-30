#pragma once
#include <vector>
#include "Enemy.h"
#include "Bullet.h"
#include "Skyscraper.h"
#include "SpriteSheet.h"

class EnemyManager {
public:
    EnemyManager();
    void Update(float dt);
    // Optionally provide a SpriteSheet to draw enemies from. If sheet==nullptr, falls back
    // to using enemyTexture or simple rects.
    void Render(SDL_Renderer* renderer, SDL_Texture* enemyTexture, SpriteSheet* sheet = nullptr);
    // Render background elements (skyscrapers) so they draw behind bullets/player
    void RenderBackground(SDL_Renderer* renderer);
    void FireRandomBullet(std::vector<Bullet>& enemyBullets);
    std::vector<Enemy> enemies;
    std::vector<Skyscraper> defenseBlocks;
    void LoadLevel(int levelIndex = 0);
private:
    float direction = 1.0f; // 1 = derecha, -1 = izquierda
    float speed = 150.0f;   // Aumentado de 50 a 150
    float dropTimer = 0.0f;
    float moveTimer = 0.0f;
    float shootTimer = 0.0f;
};
