#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <random>
#include "SpriteSheet.h"

struct PowerUp {
    enum class Type { RestoreDefense, BulletTime, ExtraLife, HomingMissiles, Shield, ContinueFire };
    SDL_FRect rect;
    float vy = 120.0f; // velocidad de caída px/s
    Type type = Type::RestoreDefense;
    bool active = true;

    PowerUp(float x=0, float y=0, Type t=Type::RestoreDefense) : type(t) {
    // Use 32x32 for scale 4 (8x8 tiles scaled x4 -> 32x32)
    rect = { x, y, 32.0f, 32.0f };
        active = true;
        spawnAbsTime = 0.0; // will be set by Game::SpawnPowerUp when spawned
    }

    // Absolute epoch seconds when spawned (for pickup time measurement)
    double spawnAbsTime = 0.0;

    void Update(float dt) {
        if (!active) return;
        rect.y += vy * dt;
        if (rect.y > 600.0f) active = false;
    }

    void Render(SDL_Renderer* renderer) {
        if (!active) return;

        // Lazy-load the miscellaneous sprite sheet once.
        static SpriteSheet miscSheet;
        static bool miscLoaded = false;
        if (!miscLoaded) {
            // Path used elsewhere in repo (tools/spritetest). Tile size default 8x8.
            std::string path = "assets/sprites/SpaceShooterAssetPack_Miscellaneous.png";
            miscLoaded = miscSheet.Load(renderer, path, 8, 8);
            if (!miscLoaded) {
                // If loading fails, fallback to previous rectangle rendering with colors.
                miscLoaded = false;
            }
        }

        // Map PowerUp::Type to sprite sheet indices per user request:
        // 0 -> RestoreDefense
        // 2 -> ExtraLife
        // 3 -> Shield
        // 4 -> BulletTime
        // 5 -> Homing/auto-aim
        // 6 -> ContinueFire
        // 1 -> random/unused
        int mappedIndex = 0;
        switch (type) {
            case Type::RestoreDefense: mappedIndex = 0; break;
            case Type::ExtraLife: mappedIndex = 2; break;
            case Type::Shield: mappedIndex = 3; break;
            case Type::BulletTime: mappedIndex = 4; break;
            case Type::HomingMissiles: mappedIndex = 5; break;
            case Type::ContinueFire: mappedIndex = 6; break;
            default: mappedIndex = 0; break;
        }

        // If the mapping would be index 1 (random), pick a random valid index instead
        if (mappedIndex == 1) {
            static std::mt19937 rng((unsigned)std::random_device{}());
            // choose among the known power-up indices (0,2,3,4,5,6)
            int choices[] = {0,2,3,4,5,6};
            std::uniform_int_distribution<int> d(0, 5);
            mappedIndex = choices[d(rng)];
        }

        if (miscLoaded && miscSheet.GetTexture()) {
            SDL_Rect src = miscSheet.GetSrcRect(mappedIndex);
            SDL_FRect srcF = {(float)src.x, (float)src.y, (float)src.w, (float)src.h};
            // Render with integer pixel scaling: draw as 16x16 (2x for 8x8 tiles)
            // Render at 32x32 (8x8 tiles scaled x4 -> 32x32)
            const float dstW = 32.0f;
            const float dstH = 32.0f;
            SDL_FRect dstF = { rect.x + (rect.w - dstW) / 2.0f, rect.y + (rect.h - dstH) / 2.0f, dstW, dstH };
            SDL_RenderTexture(renderer, miscSheet.GetTexture(), &srcF, &dstF);
        } else {
            // Fallback: original colored rectangle rendering
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
                case Type::ContinueFire:
                    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); // magenta
                    break;
            }
            SDL_RenderFillRect(renderer, &rect);
        }
    }
};
