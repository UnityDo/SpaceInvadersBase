#include "Enemy.h"

Enemy::Enemy(float x, float y) {
    rect = { x, y, 40, 20 };
}

void Enemy::Update(float dt) {
    // Movimiento enemigo
}

void Enemy::Render(SDL_Renderer* renderer) {
    if (alive) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}
