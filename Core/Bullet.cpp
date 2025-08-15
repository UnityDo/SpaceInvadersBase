#include "Bullet.h"
#include "EnemyManager.h"
#include <cmath>

Bullet::Bullet(float x, float y, float speed, float vx, bool isHoming) : speed(speed), vx(vx), isHoming(isHoming) {
    rect = { x, y, 5, 15 };
    if (isHoming) homingLife = 300; // duración en frames de homing, ~5s a 60fps
}

void Bullet::Update(float dt) {
    // Homing behavior: buscar enemigo más cercano verticalmente hacia arriba
    if (isHoming && homingLife > 0) {
        // Intentaremos encontrar el enemigo más cercano en el mundo
        // Para no depender de singletons, buscaremos en un EnemyManager global si existe (se asume que quien llama puede setear targetVX)
        // Implementación simplificada: ajustar ligeramente vx hacia el centro de pantalla cuando es homing
        float centerX = 400.0f;
        float dx = centerX - (rect.x + rect.w/2);
        float desiredVX = (dx) * 0.5f; // ganancia
        // Lerp hacia desiredVX
        vx += (desiredVX - vx) * 0.1f;
        homingLife--;
    }

    rect.y += speed * dt;
    rect.x += vx * dt;
    if (rect.y < 0 || rect.y > 600 || rect.x < 0 || rect.x > 800) active = false;
}

void Bullet::Render(SDL_Renderer* renderer) {
    if (active) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}
