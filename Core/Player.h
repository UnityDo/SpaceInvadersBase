#pragma once
#include "Entity.h"
#include "IPlayerController.h"

class Player : public Entity {
public:
    Player();
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;
    void Move(float dir, float dt);
    // Escudo del jugador
    bool shieldActive = false;
    float shieldAlpha = 0.15f; // 15% alpha
    int shieldHp = 0;
    float shieldTimer = 0.0f;
    void ShieldHit();

    // Inyectar controlador (puede ser HumanController o AIController)
    void SetController(IPlayerController* c) { controller = c; }
    IPlayerController* GetController() const { return controller; }

private:
    float speed = 600.0f; // píxeles por segundo
    IPlayerController* controller = nullptr; // no propietario
};
