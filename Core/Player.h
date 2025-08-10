#pragma once
#include "Entity.h"

class Player : public Entity {
public:
    Player();
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;
    void Move(float dx);
};
