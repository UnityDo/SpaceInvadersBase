#pragma once
#include "Entity.h"

class Enemy : public Entity {
public:
    Enemy(float x, float y);
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;
    bool alive = true;
};
