#pragma once
#include <SDL3/SDL.h>

class Renderer {
public:
    Renderer();
    ~Renderer();
    bool Init();
    void Clear();
    void Present();
    SDL_Renderer* GetSDLRenderer();
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
};
