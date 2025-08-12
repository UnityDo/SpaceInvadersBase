#include "Enemy.h"


Enemy::Enemy(float x, float y, int hp, EnemyColor color_)
    : health(hp), color(color_)
{
    rect = { x, y, 40, 20 };
}

void Enemy::Update(float dt) {
    // Movimiento enemigo
}

void Enemy::Render(SDL_Renderer* renderer) {
    if (alive) {
        // Refuerzo: asegurar que el color es el correcto
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    }
}
