#include "Bullet.h"

Bullet::Bullet(float x, float y, float speed) : speed(speed) {
    rect = { x, y, 5, 15 };
}

void Bullet::Update(float dt) {
    rect.y += speed * dt;
    if (rect.y < 0 || rect.y > 600) active = false;
}

void Bullet::Render(SDL_Renderer* renderer) {
    if (active) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}
