#include "TextRenderer.h"
#include <iostream>

TextRenderer::TextRenderer() : font(nullptr), initialized(false) {}

TextRenderer::~TextRenderer() {
    Shutdown();
}

bool TextRenderer::Init() {
    // Inicializar TTF
    if (!TTF_Init()) {
        std::cout << "Error TTF_Init: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Cargar fuente - intentar desde la carpeta fonts
    font = TTF_OpenFont("fonts/arial.ttf", 24);
    if (!font) {
        // Si no está en fonts/, intentar ruta relativa
        font = TTF_OpenFont("arial.ttf", 24);
        if (!font) {
            std::cout << "Error cargar fuente: " << SDL_GetError() << std::endl;
            TTF_Quit();
            return false;
        }
    }
    
    initialized = true;
    return true;
}

void TextRenderer::Shutdown() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    if (initialized) {
        TTF_Quit();
        initialized = false;
    }
}

void TextRenderer::RenderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
    if (!font || !initialized) return;
    
    SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), 0, color);
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        if (tex) {
            float w, h;
            SDL_GetTextureSize(tex, &w, &h);
            SDL_FRect rect = {(float)x, (float)y, w, h};
            SDL_RenderTexture(renderer, tex, nullptr, &rect);
            SDL_DestroyTexture(tex);
        }
        SDL_DestroySurface(surf);
    }
}
