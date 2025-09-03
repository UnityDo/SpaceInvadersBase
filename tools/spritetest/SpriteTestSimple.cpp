// SpriteTestSimple.cpp - Versión mínima que usa la misma configuración que Game.cpp
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>
#include "../Core/Renderer.h"
#include "../Core/SpriteSheet.h"
#include "../Core/TextRenderer.h"

const int WIN_W = 640;
const int WIN_H = 480;

int main() {
    // Misma inicialización que Game.cpp
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("No se pudo inicializar SDL: %s", SDL_GetError());
        return -1;
    }
    
    // Usar la clase Renderer igual que el juego principal
    Renderer* renderer = new Renderer();
    if (!renderer->Init()) {
        std::cerr << "Failed to init renderer\n";
        delete renderer;
        SDL_Quit();
        return -1;
    }
    
    std::cout << "SDL and Renderer initialized successfully!\n";
    
    // Inicializar TextRenderer igual que en Game.cpp
    TextRenderer* textRenderer = new TextRenderer();
    if (!textRenderer->Init()) {
        std::cout << "Warning: No se pudo inicializar TextRenderer" << std::endl;
    }
    
    // Cargar sprite sheet
    SpriteSheet sheet;
    //std::string sheetPath = "assets/sprites/SpaceShooterAssetPack_Ships.png";
    std::string sheetPath = "assets/sprites/SpaceShooterAssetPack_Miscellaneous.png";
   
    if (!sheet.Load(renderer->GetSDLRenderer(), sheetPath, 8, 8)) {
        std::cout << "Failed to load sprite sheet, continuing with test rectangle...\n";
    } else {
        std::cout << "Sprite sheet loaded successfully!\n";
        // Configurar filtrado nearest-neighbor para pixel art
        SDL_SetTextureScaleMode(sheet.GetTexture(), SDL_SCALEMODE_NEAREST);
    }
    
    int spriteIndex = 0;
    int scale = 5;
    bool running = true;
    
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_ESCAPE || e.key.key == SDLK_Q) running = false;
                if (e.key.key == SDLK_RIGHT) spriteIndex++;
                if (e.key.key == SDLK_LEFT && spriteIndex > 0) spriteIndex--;
                if (e.key.key >= SDLK_0 && e.key.key <= SDLK_9) {
                    spriteIndex = e.key.key - SDLK_0;
                }
                // Controles para cambiar scale
                if (e.key.key == SDLK_PLUS || e.key.key == SDLK_EQUALS) {
                    if (scale < 20) scale++;
                }
                if (e.key.key == SDLK_MINUS) {
                    if (scale > 1) scale--;
                }
            }
        }
        
        // Limpiar pantalla
        renderer->Clear();
        
        // Dibujar sprite en el centro
        if (sheet.GetTexture()) {
            SDL_Rect srcRect = sheet.GetSrcRect(spriteIndex);
            int dstW = srcRect.w * scale;
            int dstH = srcRect.h * scale;
            int dstX = (WIN_W - dstW) / 2;
            int dstY = (WIN_H - dstH) / 2;
            
            SDL_FRect srcF = {(float)srcRect.x, (float)srcRect.y, (float)srcRect.w, (float)srcRect.h};
            SDL_FRect dstF = {(float)dstX, (float)dstY, (float)dstW, (float)dstH};
            
            SDL_RenderTexture(renderer->GetSDLRenderer(), sheet.GetTexture(), &srcF, &dstF);
        } else {
            // Fallback: dibujar rectángulo blanco
            SDL_SetRenderDrawColor(renderer->GetSDLRenderer(), 255, 255, 255, 255);
            SDL_FRect rect = {WIN_W/2 - 40, WIN_H/2 - 40, 80, 80};
            SDL_RenderFillRect(renderer->GetSDLRenderer(), &rect);
        }
        
        // Renderizar el índice actual
        if (textRenderer) {
            SDL_Color white = {255, 255, 255, 255};
            std::string indexText = "Index: " + std::to_string(spriteIndex);
            std::string scaleText = "Scale: " + std::to_string(scale) + "x (Size: " + std::to_string(8*scale) + "x" + std::to_string(8*scale) + ")";
            textRenderer->RenderText(renderer->GetSDLRenderer(), indexText, 10, 10, white);
            textRenderer->RenderText(renderer->GetSDLRenderer(), scaleText, 10, 30, white);
            textRenderer->RenderText(renderer->GetSDLRenderer(), "Arrows: change sprite, Numbers: direct index", 10, 50, white);
            textRenderer->RenderText(renderer->GetSDLRenderer(), "+/-: change scale, Q/ESC: quit", 10, 70, white);
        }
        
        renderer->Present();
        SDL_Delay(16); // ~60 FPS
    }
    
    std::cout << "Test completed successfully!\n";
    
    // Cleanup
    if (textRenderer) {
        textRenderer->Shutdown();
        delete textRenderer;
    }
    delete renderer;
    SDL_Quit();
    return 0;
}
