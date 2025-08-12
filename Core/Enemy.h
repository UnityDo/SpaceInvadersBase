#pragma once
#include "Entity.h"
#include "IEnemy.h" 

struct EnemyColor {
    Uint8 r, g, b, a;
    EnemyColor(Uint8 r_, Uint8 g_, Uint8 b_, Uint8 a_ = 255) : r(r_), g(g_), b(b_), a(a_) {}
};

class Enemy : public Entity, public IEnemy {
public:
    Enemy(float x, float y, int hp = 1, EnemyColor color = EnemyColor(255,0,0,255));
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;
    void TakeDamage(int amount) override { /* ... */ }
    bool IsAlive() const override { return alive; }
    bool alive = true;
    int health = 1;
    EnemyColor color;
};
