#include "InputManager.h"
#include <SDL3/SDL.h>
#include <iostream>

extern bool g_running;

void InputManager::Update() {
    // Usar el estado actual del teclado para permitir combinaciones (mover y disparar simultáneo)
    const auto state = SDL_GetKeyboardState(nullptr);
    left = state[SDL_SCANCODE_LEFT];
    right = state[SDL_SCANCODE_RIGHT];
    fire = state[SDL_SCANCODE_SPACE];
}

void InputManager::HandleEvent(const SDL_Event& event) {
    // Mantener compatibilidad con eventos; no limpiar en key up para no sobrescribir el estado de SDL_GetKeyboardState
    if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_LEFT) {
            left = true;
        }
        if (event.key.key == SDLK_RIGHT) {
            right = true;
        }
        if (event.key.key == SDLK_SPACE) {
            fire = true;
        }
    } else if (event.type == SDL_EVENT_KEY_UP) {
        if (event.key.key == SDLK_LEFT) {
            left = false;
        }
        if (event.key.key == SDLK_RIGHT) {
            right = false;
        }
        if (event.key.key == SDLK_SPACE) {
            fire = false;
        }
    }
}

bool InputManager::IsLeftPressed() const { return left; }
bool InputManager::IsRightPressed() const { return right; }
bool InputManager::IsFirePressed() const { return fire; }
