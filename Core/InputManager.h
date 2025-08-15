#pragma once
#include <SDL3/SDL.h>

class InputManager {
public:
    void Update();
    void HandleEvent(const SDL_Event& event);
    bool IsLeftPressed() const;
    bool IsRightPressed() const;
    bool IsFirePressed() const;
private:
    bool left = false;
    bool right = false;
    bool fire = false;
};
