#pragma once
#include "Entity.h"

class Bullet : public Entity {
public:
    Bullet(float x, float y, float speed);
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;
    bool active = true;
    float speed;
};
