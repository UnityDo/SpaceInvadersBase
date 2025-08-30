#include "Enemy.h"
#include <cmath>
#include <cstdlib>



Enemy::Enemy(float x, float y, int hp, EnemyColor color_, EnemyType type_, float speed_, int damage_, MovePattern pattern_)
    : health(hp), maxHealth(hp), damage(damage_), speed(speed_), color(color_), type(type_), pattern(pattern_)
{
    rect = { x, y, 40, 20 };
    isSplitter = (type == EnemyType::Splitter);
    isBoss = (type == EnemyType::Boss);
    isSniper = (type == EnemyType::Sniper);
}

void Enemy::Update(float dt) {
    if (!alive) return;
    patternTimer += dt;

    // Logic shared for bosses: decisión periódica
    if (isBoss) {
        decisionTimer += dt;
        if (decisionTimer >= decisionInterval) {
            decisionTimer = 0.0f;
            int r = rand() % 100;
            // 65% maneuver
            if (r < 65) {
                bossAction = BossAction::Maneuver;
                actionDuration = 1.0f + (rand() % 200) / 100.0f; // 1.0 - 3.0s aprox
                actionTimer = actionDuration;
            }
            // 20% teleport
            else if (r < 85) {
                // Planificar teleport a posición aleatoria horizontal manteniendo y
                const float screenW = 800.0f;
                float margin = 20.0f;
                float maxX = screenW - rect.w - margin;
                float newX = margin + (rand() / (float)RAND_MAX) * (maxX - margin);
                // No mover aún; guardar solicitud de teleport para que EnemyManager verifique huecos libres
                pendingTeleportX = newX;
                wantsTeleport = true;
                bossAction = BossAction::None;
            }
            // 10% triple shot
            else if (r < 95) {
                bossAction = BossAction::TripleShot;
                actionTimer = 0.5f; // ventana corta para que EnemyManager dispare
            }
            // restante 5% nada
        }
    }

    switch (type) {
        case EnemyType::Basic:
            // Movimiento según patrón
            switch (pattern) {
                case MovePattern::Straight:
                    rect.y += speed * 30 * dt;
                    break;
                case MovePattern::ZigZag:
                    rect.x += std::sin(patternTimer * 2.5f) * 60 * dt;
                    rect.y += speed * 20 * dt;
                    break;
                case MovePattern::Diagonal:
                    rect.x += speed * 20 * dt;
                    rect.y += speed * 20 * dt;
                    break;
                default: break;
            }
            break;
        case EnemyType::Fast:
            switch (pattern) {
                case MovePattern::ZigZag:
                    rect.x += std::sin(patternTimer * 5.0f) * 120 * dt;
                    rect.y += speed * 60 * dt;
                    break;
                case MovePattern::Dive:
                    rect.y += speed * 120 * dt;
                    break;
                default: break;
            }
            break;
        case EnemyType::Tank:
            switch (pattern) {
                case MovePattern::Straight:
                    rect.y += speed * 10 * dt;
                    break;
                case MovePattern::DescendStopShoot:
                    if (rect.y < 200) rect.y += speed * 20 * dt;
                    // luego se queda quieto y dispara
                    break;
                default: break;
            }
            break;
        case EnemyType::Boss:
            // Ejecutar acción si hay alguna
            if (bossAction == BossAction::Maneuver && actionTimer > 0.0f) {
                // Zigzag evasivo: usar sin para moverse horizontalmente rápido
                rect.x += std::sin(patternTimer * 6.0f) * 160 * dt;
                // pequeño desplazamiento vertical para simular maniobra
                rect.y += std::sin(patternTimer * 3.0f) * 8.0f * dt;
                actionTimer -= dt;
                if (actionTimer <= 0.0f) bossAction = BossAction::None;
            } else {
                // Comportamiento por patrón cuando no está maniobrando
                switch (pattern) {
                    case MovePattern::Circle:
                        rect.x += std::cos(patternTimer) * 80 * dt;
                        rect.y += std::sin(patternTimer) * 40 * dt;
                        break;
                    case MovePattern::ZigZag:
                        rect.x += std::sin(patternTimer * 2.0f) * 100 * dt;
                        break;
                    default: break;
                }
            }
            // actionTimer para triple shot se deja para que EnemyManager lo detecte
            break;
        case EnemyType::Sniper:
            switch (pattern) {
                case MovePattern::Stationary:
                    // Quieto
                    break;
                case MovePattern::ZigZag:
                    rect.x += std::sin(patternTimer * 1.5f) * 40 * dt;
                    break;
                default: break;
            }
            break;
        case EnemyType::Splitter:
            switch (pattern) {
                case MovePattern::ZigZag:
                    rect.x += std::sin(patternTimer * 2.0f) * 60 * dt;
                    rect.y += speed * 30 * dt;
                    break;
                case MovePattern::Scatter:
                    rect.x += (phase == 0 ? -1 : 1) * speed * 40 * dt;
                    rect.y += speed * 30 * dt;
                    break;
                default: break;
            }
            break;
        default: break;
    }
}

void Enemy::Render(SDL_Renderer* renderer) {
    if (!alive) return;

    // Try to draw a sprite if the Renderer provided one via renderer's user data.
    // The project has a Renderer class that stores the texture; as a lightweight approach
    // we attempt to query a texture by getting the SDL_Renderer associated texture through
    // a custom function. Since this file doesn't have direct access to Renderer instance,
    // we'll attempt to find a texture named "enemy" stored in the renderer's userdata pointer
    // (set elsewhere). If not found, fallback to rect rendering.

    // The project's Renderer class exposes GetEnemyTexture() and the Game code uses a single
    // global Renderer; to keep changes minimal, we'll check for an attached texture via
    // SDL_GetRendererInfo userdata is not reliable, so instead we accept a convention: the
    // Renderer sets the SDL_Renderer user data to an SDL_Texture* for enemy under key "ENEMY_TEX"
    // If that is not the case, fallback.

    // Fallback: simple filled rect
    SDL_Texture* tex = nullptr;
    // try to retrieve a pointer stored as renderer's draw color userdata (non-standard),
    // but since we can't guarantee, we'll simply attempt to render the rect if no texture is set.

    // NOTE: Downstream we will update Game to pass the Renderer instance or expose texture.
    // For now: normal filled rect (maintain prior behavior).
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
    if (isBoss) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_FRect outline = { rect.x - 2.0f, rect.y - 2.0f, rect.w + 4.0f, rect.h + 4.0f };
        SDL_RenderRect(renderer, &outline);
    }
}

void Enemy::TakeDamage(int amount) {
    if (!alive) return;
    health -= amount;
    if (health <= 0) {
        alive = false;
        // Aquí se puede manejar lógica especial: splitter, boss, etc.
    }
}
