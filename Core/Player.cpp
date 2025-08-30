#include "Player.h"

Player::Player() {
    // Updated to match player sprite size (45x43)
    rect = { 400, 550, 45, 43 };
    speed = 300.0f; // velocidad en píxeles por segundo
}

void Player::Update(float dt) {
    // Si hay un controlador, dejar que lo actualice (p. ej. IA)
    if (controller) controller->Update(dt);

    // Actualizar temporizador de escudo
    if (shieldActive) {
        shieldTimer -= dt;
        if (shieldTimer <= 0.0f) {
            shieldActive = false;
            shieldHp = 0;
        }
    }
}

void Player::Render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &rect);

    // Dibujar escudo si activo con alpha
    if (shieldActive) {
        Uint8 a = static_cast<Uint8>(255 * shieldAlpha);
        SDL_SetRenderDrawColor(renderer, 0, 191, 255, a); // color cyan claro con alpha
        SDL_FRect s = { rect.x - 5.0f, rect.y - 5.0f, rect.w + 10.0f, rect.h + 10.0f };
        SDL_RenderFillRect(renderer, &s);
    }
}

void Player::Move(float dir, float dt) {
    // dir es -1 (izquierda), 0, 1 (derecha)
    float dx = dir * speed * dt;
    rect.x += dx;

    // Limitar al ancho de la ventana para evitar que el jugador se salga
    if (rect.x < 0.0f) rect.x = 0.0f;
    const float screenW = 800.0f;
    if (rect.x + rect.w > screenW) rect.x = screenW - rect.w;
}

void Player::ShieldHit() {
    if (!shieldActive) return;
    shieldHp--;
    if (shieldHp <= 0) {
        shieldActive = false;
        shieldTimer = 0.0f;
    } else {
        // reducir alpha progresivamente
        shieldAlpha *= 0.6f;
    }
}
