#pragma once
#include <SDL3/SDL.h>

struct PowerUp {
    enum class Type { RestoreDefense, BulletTime, ExtraLife, HomingMissiles, Shield };
    SDL_FRect rect;
    float vy = 120.0f; // velocidad de caída px/s
    Type type = Type::RestoreDefense;
    bool active = true;

    PowerUp(float x=0, float y=0, Type t=Type::RestoreDefense) : type(t) {
        rect = { x, y, 18.0f, 18.0f };
        active = true;
    }

    void Update(float dt) {
        if (!active) return;
        rect.y += vy * dt;
        if (rect.y > 600.0f) active = false;
    }

    void Render(SDL_Renderer* renderer) {
        if (!active) return;
        // Color según tipo
        switch (type) {
            case Type::RestoreDefense:
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // cyan
                break;
            case Type::BulletTime:
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // blue
                break;
            case Type::ExtraLife:
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // green
                break;
            case Type::HomingMissiles:
                SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); // orange
                break;
            case Type::Shield:
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // yellow
                break;
        }
        SDL_RenderFillRect(renderer, &rect);
    }
};
