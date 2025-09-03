#include "Bullet.h"
#include "EnemyManager.h"
#include <cmath>

Bullet::Bullet(float x, float y, float speed, float vx, bool isHoming, float tX, float tY, Owner owner, bool small) : speed(speed), vx(vx), isHoming(isHoming), targetX(tX), targetY(tY), owner(owner), smallForContinueFire(small) {
    // Default size for enemy bullets: 5x15, player bullets normally 5x15
    // If smallForContinueFire is true, use a slimmer/shorter bullet (e.g., 3x12)
    if (smallForContinueFire) {
        rect = { x, y, 3, 12 };
    } else {
        rect = { x, y, 5, 15 };
    }
    if (isHoming) {
        homingLife = 300; // duración en frames de homing, ~5s a 60fps
        if (targetX >= 0.0f && targetY >= 0.0f) hasTarget = true;
        // homing más lento que un misil normal
        // limitamos la velocidad vertical a un valor menor (velocidad es positiva hacia abajo o negativa hacia arriba)
    }
}

void Bullet::Update(float dt) {
    // Homing behavior: ajustar vx para dirigirse al target sin invertir vy
    if (isHoming && homingLife > 0) {
        float bx = rect.x + rect.w / 2.0f;
        float by = rect.y + rect.h / 2.0f;
        float tx = targetX;
        float ty = targetY;
        // Si no tenemos target explícito, no hacemos nada agresivo
        if (!hasTarget) {
            // comportamiento por defecto antiguo: tirar ligeramente al centro
            tx = 400.0f;
            ty = 0.0f;
        }

        float dx = tx - bx;
        float dy = ty - by;

        // Calcular tiempo aproximado para alcanzar verticalmente, usando la velocidad vertical actual (speed)
        float vy = speed; // note: speed puede ser negativo para disparos del jugador
        float absVy = fabsf(vy);
        if (absVy < 1.0f) absVy = 1.0f;
        float t = fabsf(dy) / absVy;
        if (t < 0.001f) t = 0.5f;

        // Desired horizontal speed para alcanzar al target en tiempo t
        float desiredVX = dx / t;

        // Limitar para que no retroceda: no permitir desiredVX que haga que el misil invierta su dirección vertical
        // En este contexto consideramos "no retroceder" como no invertir la componente vertical; ya que vy controla vertical,
        // nos aseguramos de que el ajuste horizontal no haga cosas extrañas.

        // Limitar velocidad horizontal a un máximo razonable
        if (desiredVX > maxHomingHorizSpeed) desiredVX = maxHomingHorizSpeed;
        if (desiredVX < -maxHomingHorizSpeed) desiredVX = -maxHomingHorizSpeed;

        // Aplicar un lerp suave para que el giro sea lento
        vx += (desiredVX - vx) * homingTurnLerp;

        homingLife--;
    }

    // Mover usando speed (vertical) y vx (horizontal)
    rect.y += speed * dt;
    rect.x += vx * dt;
    if (rect.y < 0 || rect.y > 600 || rect.x < 0 || rect.x > 800) active = false;
}

void Bullet::Render(SDL_Renderer* renderer) {
    if (active) {
        if (owner == Owner::Player) {
            if (smallForContinueFire) {
                // Color #66cc99 (102,204,153)
                SDL_SetRenderDrawColor(renderer, 102, 204, 153, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
        } else {
            // enemy bullets remain red
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        }
        SDL_RenderFillRect(renderer, &rect);
    }
}
