#include "InputManager.h"
#include <SDL3/SDL.h>
#include <iostream>

extern bool g_running;

void InputManager::Update() {
    // Limpiar el estado de las teclas al inicio de cada frame
    left = right = fire = false;
}

void InputManager::HandleEvent(const SDL_Event& event) {
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
    }
}

bool InputManager::IsLeftPressed() const { return left; }
bool InputManager::IsRightPressed() const { return right; }
bool InputManager::IsFirePressed() const { return fire; }
