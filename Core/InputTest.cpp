#include <SDL3/SDL.h>
#include <iostream>

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Test Input", 400, 300, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    
    std::cout << "Test Input - Presiona flechas izq/der, ESC para salir" << std::endl;
    
    bool running = true;
    int x = 200;
    
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN) {
                std::cout << "Tecla presionada: " << e.key.key << std::endl;
                if (e.key.key == SDLK_LEFT) {
                    x -= 10;
                    std::cout << "Izquierda! x=" << x << std::endl;
                }
                if (e.key.key == SDLK_RIGHT) {
                    x += 10;
                    std::cout << "Derecha! x=" << x << std::endl;
                }
                if (e.key.key == SDLK_ESCAPE) running = false;
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_FRect rect = {(float)x, 150, 20, 20};
        SDL_RenderFillRect(renderer, &rect);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
