#pragma once
#include "Entity.h"

class Bullet : public Entity {
public:
    Bullet(float x, float y, float speed, float vx = 0.0f, bool isHoming = false);
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;
    bool active = true;
    float speed;
    float vx = 0.0f;
    bool isHoming = false;
    int homingLife = 0; // cuantos frames/actualizaciones mantiene comportamiento homing
};
