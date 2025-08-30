#include "Renderer.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h> // Make sure this path is correct for your setup

Renderer::Renderer() : window(nullptr), renderer(nullptr) {}

Renderer::~Renderer() {
    // Free loaded textures and quit SDL_image
    if (enemyTexture) SDL_DestroyTexture(enemyTexture);
    if (playerTexture) SDL_DestroyTexture(playerTexture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
}
bool Renderer::Init() {
    window = SDL_CreateWindow("Space Invaders", 800, 600, 0);
    renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer) {
        // Habilitar blending por defecto para poder dibujar colores con alfa
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        // Intentar cargar texturas necesarias
        if (!LoadTexture("assets/base.png", enemyTexture)) {
            std::cerr << "[Renderer] Warning: failed to load assets/base.png, enemies will be drawn as rects" << std::endl;
        }
        // Keep existing single-texture fallback
        if (!LoadTexture("assets/player.png", playerTexture)) {
            std::cerr << "[Renderer] Warning: failed to load assets/player.png, player will be drawn as rect" << std::endl;
        }
        // Try to load player sprite sheet (8x8 tiles)
        hasPlayerSheet = playerSheet.Load(renderer, "assets/sprites/SpaceShooterAssetPack_Ships.png", 8, 8);
        if (!hasPlayerSheet) {
            std::cerr << "[Renderer] Warning: failed to load player sprite sheet, falling back to player.png or rect" << std::endl;
        }
    }
    return window && renderer;
}


void Renderer::Clear() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void Renderer::Present() {
    SDL_RenderPresent(renderer);
}

SDL_Renderer* Renderer::GetSDLRenderer() {
    return renderer;
}

SDL_Texture* Renderer::GetEnemyTexture() {
    return enemyTexture;
}

SDL_Texture* Renderer::GetPlayerTexture() {
    return playerTexture;
}

// Access to player sprite sheet
SpriteSheet* Renderer::GetPlayerSheet() {
    return hasPlayerSheet ? &playerSheet : nullptr;
}

bool Renderer::HasPlayerSheet() {
    return hasPlayerSheet;
}

bool Renderer::LoadTexture(const std::string& path, SDL_Texture*& outTex) {
    outTex = nullptr;
    if (!renderer) return false;
    // Use IMG_LoadTexture to create the texture directly from file (no surface needed)
    outTex = IMG_LoadTexture(renderer, path.c_str());
    return outTex != nullptr;
}
