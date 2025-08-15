#pragma once
#include "Entity.h"

class Player : public Entity {
public:
    Player();
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;
    void Move(float dir, float dt);
    // Escudo del jugador
    bool shieldActive = false;
    float shieldAlpha = 0.15f; // 15% alpha
    int shieldHp = 0;
    float shieldTimer = 0.0f;
    void ShieldHit();

    private:
    float speed = 600.0f; // píxeles por segundo
};
