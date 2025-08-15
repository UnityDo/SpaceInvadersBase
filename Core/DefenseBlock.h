#pragma once
#include <SDL3/SDL.h>

struct DefenseBlock {
    SDL_FRect rect;
    SDL_FRect originalRect;
    int hp = 3; // tres impactos de bala enemiga
    bool alive = true;

    DefenseBlock(float x=0, float y=0, float w=60, float h=20) {
        rect = {x,y,w,h};
        originalRect = rect;
        hp = 3;
        alive = true;
    }

    void TakeBulletHit() {
        if (!alive) return;
        hp--;
        if (hp <= 0) {
            alive = false;
            return;
        }
        float factor = (float)hp / 3.0f;
        rect.w = originalRect.w * factor;
        rect.h = originalRect.h * factor;
        rect.x = originalRect.x + (originalRect.w - rect.w) / 2.0f;
        rect.y = originalRect.y + (originalRect.h - rect.h) / 2.0f;
    }

    void DestroyByEnemy() {
        alive = false;
    }

    void Render(SDL_Renderer* renderer) {
        if (!alive) return;
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // amarillo
        SDL_RenderFillRect(renderer, &rect);
    }
};
