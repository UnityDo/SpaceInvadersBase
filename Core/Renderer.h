#pragma once
#include <SDL3/SDL.h>
#include <string>
#include "SpriteSheet.h"

class Renderer {
public:
    Renderer();
    ~Renderer();
    bool Init();
    void Clear();
    void Present();
    SDL_Renderer* GetSDLRenderer();
    // Textures
    SDL_Texture* GetEnemyTexture();
    SDL_Texture* GetPlayerTexture();
    // Sprite sheet accessors
    SpriteSheet* GetPlayerSheet();
    bool HasPlayerSheet();

private:
    bool LoadTexture(const std::string& path, SDL_Texture*& outTex);

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* enemyTexture = nullptr; // texture for enemies (base.png)
    SDL_Texture* playerTexture = nullptr; // texture for player (player.png)
    // Sprite sheet for player (8x8 tiles)
    SpriteSheet playerSheet;
    bool hasPlayerSheet = false;
};
