#pragma once
#include "Entity.h"

class Bullet : public Entity {
public:
    enum class Owner { Player, Enemy };
    // targetX/targetY opcionales para misiles homing (si targetX >= 0 entonces se usa)
    // smallForContinueFire -> indica que la bala es más pequeña y se colorea distinto (power-up ContinueFire)
    Bullet(float x, float y, float speed, float vx = 0.0f, bool isHoming = false, float targetX = -1.0f, float targetY = -1.0f, Owner owner = Owner::Player, bool smallForContinueFire = false);
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;
    bool active = true;
    float speed;
    float vx = 0.0f;
    bool isHoming = false;
    int homingLife = 0; // cuantos frames/actualizaciones mantiene comportamiento homing
    // Target explícito para homing
    float targetX = -1.0f;
    float targetY = -1.0f;
    bool hasTarget = false;
    float maxHomingHorizSpeed = 220.0f; // velocidad horizontal máxima del homing
    float homingTurnLerp = 0.06f; // velocidad de corrección (menor = giro más lento)
    Owner owner = Owner::Player;
    bool smallForContinueFire = false;
};
