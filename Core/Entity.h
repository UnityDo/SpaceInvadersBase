#pragma once
#include <SDL3/SDL.h>
class Entity {
public:
    virtual ~Entity() {}
    virtual void Update(float dt) = 0;
    virtual void Render(SDL_Renderer* renderer) = 0;
    SDL_FRect rect;
};
