#include "Player.h"

Player::Player() {
    rect = { 400, 550, 50, 20 };
}

void Player::Update(float dt) {
    // Movimiento y lógica del jugador
}

void Player::Render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void Player::Move(float dx) {
    rect.x += dx;
}
