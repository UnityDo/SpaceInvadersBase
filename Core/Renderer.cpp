#include "Renderer.h"

Renderer::Renderer() : window(nullptr), renderer(nullptr) {}

Renderer::~Renderer() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
}

bool Renderer::Init() {
    window = SDL_CreateWindow("Space Invaders", 800, 600, 0);
    renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer) {
        // Habilitar blending por defecto para poder dibujar colores con alfa
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
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
