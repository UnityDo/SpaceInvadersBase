#include "EnemyManager.h"
#include "EnemyFactory.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

EnemyManager::EnemyManager() {
    LoadLevel(0); // Cargar nivel 1 por defecto
    std::cout << "[EnemyManager] Enemigos tras LoadLevel: " << enemies.size() << std::endl;
    if (!enemies.empty()) {
        int vivos = 0;
        for (const auto& e : enemies) if (e.alive) vivos++;
        std::cout << "[EnemyManager] Enemigos vivos al inicio: " << vivos << std::endl;
    }
    srand((unsigned)time(nullptr));
}

void EnemyManager::LoadLevel(int levelIndex) {
    enemies = EnemyFactory::CreateEnemiesFromLevels("Data/levels.json", levelIndex);
    // Crear defensas estándar: dos líneas de bloques solo si no existen (persisten entre niveles)
    // defenseBlocks.clear();

    // Solo crear defensas si todavía no hay (persisten entre niveles)
    if (defenseBlocks.empty()) {
        const float screenWidth = 800.0f; // Coincide con Renderer::Init()
        const float margin = 40.0f; // margen lateral
        const float blockW = 80.0f;
        const float blockH = 20.0f;
        const int columns = 6; // aumentar columnas para cubrir laterales

        float gap = 10.0f;
        if (columns > 1) {
            gap = (screenWidth - 2.0f * margin - columns * blockW) / (columns - 1);
            if (gap < 8.0f) gap = 8.0f; // separación mínima
        }

        for (int row = 0; row < 2; ++row) {
            for (int i = 0; i < columns; ++i) {
                float x = margin + i * (blockW + gap);
                float y = 400.0f + row * (blockH + 10.0f);
                defenseBlocks.emplace_back(x, y, blockW, blockH);
            }
        }
        std::cout << "[EnemyManager] Defense blocks created: " << defenseBlocks.size() << std::endl;
    } else {
        std::cout << "[EnemyManager] Defense blocks preserved across levels: " << defenseBlocks.size() << std::endl;
    }
}

void EnemyManager::Update(float dt) {
    moveTimer += dt;
    shootTimer += dt;
    
    // Movimiento más rápido - cada 0.3 segundos en lugar de 1 segundo
    if (moveTimer >= 0.3f) {
        moveTimer = 0.0f;
        
        // Verificar si algún enemigo toca los bordes
        bool hitEdge = false;
        for (auto& e : enemies) {
            if (e.alive) {
                if ((direction > 0 && e.rect.x >= 750) || (direction < 0 && e.rect.x <= 10)) {
                    hitEdge = true;
                    break;
                }
            }
        }
        
        // Si toca el borde, cambiar dirección y bajar
        if (hitEdge) {
            direction *= -1;
            for (auto& e : enemies) {
                if (e.alive) {
                    e.rect.y += 25; // Bajar más rápido
                }
            }
        } else {
            // Intentar mover cada enemigo horizontalmente, evitando solapamientos
            float delta = 35.0f * direction;
            auto wouldOverlap = [&](const SDL_FRect& a, const SDL_FRect& b) {
                return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
            };
            
            // Para mantener la formación, evaluamos cada enemigo individualmente
            for (size_t i = 0; i < enemies.size(); ++i) {
                auto& e = enemies[i];
                if (!e.alive) continue;
                SDL_FRect pred = e.rect;
                pred.x += delta;
                
                bool collision = false;
                // Comprobar colisión con otros enemigos vivos
                for (size_t j = 0; j < enemies.size(); ++j) {
                    if (i == j) continue;
                    auto& other = enemies[j];
                    if (!other.alive) continue;
                    if (wouldOverlap(pred, other.rect)) { collision = true; break; }
                }
                
                // Si no colisiona con otro enemigo, aplicar movimiento
                if (!collision) {
                    e.rect.x = pred.x;
                }
                // Si collision=true, dejamos al enemigo en su lugar (evita solapamiento)
            }
        }
    }
    
    for (auto& e : enemies) {
        e.Update(dt);

        // Si el boss solicitó teletransportarse, verificar hueco libre y aplicar
        if (e.alive && e.wantsTeleport) {
            auto rectWouldOverlap = [&](const SDL_FRect& a, const SDL_FRect& b) {
                return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
            };

            const float screenW = 800.0f;
            float targetX = e.pendingTeleportX;
            if (targetX < 0.0f) targetX = e.rect.x;
            SDL_FRect cand = { targetX, e.rect.y, e.rect.w, e.rect.h };

            auto overlapsAny = [&](const SDL_FRect& r) {
                // comprobar con otros enemigos
                for (auto& other : enemies) {
                    if (&other == &e) continue;
                    if (!other.alive) continue;
                    if (rectWouldOverlap(r, other.rect)) return true;
                }
                // comprobar con bloques defensivos
                for (auto& b : defenseBlocks) {
                    if (!b.alive) continue;
                    if (rectWouldOverlap(r, b.rect)) return true;
                }
                return false;
            };

            bool placed = false;
            if (!overlapsAny(cand)) {
                e.rect.x = cand.x;
                placed = true;
            } else {
                // buscar huecos a izquierda/derecha
                for (int off = 16; off <= 240 && !placed; off += 16) {
                    float lx = targetX - off;
                    float rx = targetX + off;
                    if (lx >= 0.0f) {
                        SDL_FRect r2 = { lx, e.rect.y, e.rect.w, e.rect.h };
                        if (!overlapsAny(r2)) { e.rect.x = lx; placed = true; break; }
                    }
                    if (rx + e.rect.w <= screenW) {
                        SDL_FRect r3 = { rx, e.rect.y, e.rect.w, e.rect.h };
                        if (!overlapsAny(r3)) { e.rect.x = rx; placed = true; break; }
                    }
                }
            }

            if (placed) {
                std::cout << "[EnemyManager] Boss teleported to x=" << e.rect.x << "\n";
            } else {
                std::cout << "[EnemyManager] Boss teleport requested but no free spot found\n";
            }
            // Reset request
            e.wantsTeleport = false;
            e.pendingTeleportX = -1.0f;
        }

        // Si el enemigo cruza la parte inferior de la ventana, avisar para que CollisionManager gestione la pérdida de vida
        const float screenH = 600.0f; // altura de ventana
        if (e.alive && (e.rect.y + e.rect.h >= screenH)) {
            // No marcamos e.alive = false aquí para que CollisionManager pueda detectarlo y restar la vida
            std::cout << "[EnemyManager] Enemy reached bottom at x=" << e.rect.x << " y=" << e.rect.y << ". CollisionManager will handle life loss." << std::endl;
        }
    }
}

void EnemyManager::FireRandomBullet(std::vector<Bullet>& enemyBullets) {
    // Primero verificar si algún boss ha solicitado triple shot
    for (auto& enemy : enemies) {
        if (!enemy.alive) continue;
        if (enemy.isBoss && enemy.bossAction == Enemy::BossAction::TripleShot && enemy.actionTimer > 0.0f) {
            float bulletX = enemy.rect.x + enemy.rect.w / 2 - 2.5f;
            float bulletY = enemy.rect.y + enemy.rect.h;
            // Tres balas con diferente velocidad horizontal
            enemyBullets.emplace_back(bulletX, bulletY, 220.0f, -120.0f);
            enemyBullets.emplace_back(bulletX, bulletY, 220.0f, 0.0f);
            enemyBullets.emplace_back(bulletX, bulletY, 220.0f, 120.0f);
            // Consumir la acción
            enemy.bossAction = Enemy::BossAction::None;
            enemy.actionTimer = 0.0f;
            return;
        }
    }

    // Buscar enemigos vivos en la fila inferior
    std::vector<Enemy*> bottomEnemies;
    for (auto& enemy : enemies) {
        if (!enemy.alive) continue;
        
        bool isBottom = true;
        for (auto& other : enemies) {
            if (!other.alive) continue;
            if (abs(other.rect.x - enemy.rect.x) < 30 && other.rect.y > enemy.rect.y) {
                isBottom = false;
                break;
            }
        }
        if (isBottom) {
            bottomEnemies.push_back(&enemy);
        }
    }
    
    // Disparar desde un enemigo aleatorio de la fila inferior
    if (!bottomEnemies.empty()) {
        int randomIndex = rand() % bottomEnemies.size();
        Enemy* shooter = bottomEnemies[randomIndex];
        float bulletX = shooter->rect.x + shooter->rect.w / 2 - 2.5f;
        float bulletY = shooter->rect.y + shooter->rect.h;
        enemyBullets.emplace_back(bulletX, bulletY, 200.0f); // Velocidad hacia abajo
    }
}

void EnemyManager::Render(SDL_Renderer* renderer) {
    for (auto& e : enemies) e.Render(renderer);
    for (auto& b : defenseBlocks) b.Render(renderer);
}
