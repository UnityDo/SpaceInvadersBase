#include "CollisionManager.h"
#include "Game.h"
#include "DefenseBlock.h"
#include <iostream>

CollisionManager::CollisionManager(AudioManagerMiniaudio* audio) : audioManager(audio) {
}

// Función auxiliar para detectar colisión entre dos rectángulos
bool RectCollision(const SDL_FRect& a, const SDL_FRect& b) {
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

void CollisionManager::CheckCollisions(Player& player, EnemyManager& enemies, std::vector<Bullet>& playerBullets, std::vector<Bullet>& enemyBullets, ParticleSystem& particles, Game& game) {
    // Integrar bloques defensivos (si existen)
    auto& blocks = enemies.defenseBlocks;

    // Colisiones: balas del jugador con enemigos
    for (auto& bullet : playerBullets) {
        if (!bullet.active) continue;

        bool bulletConsumed = false;
        // Primero comprobar colisión con bloques defensivos (las balas del jugador atraviesan, así que ignorar)
        for (auto& block : blocks) {
            if (!block.alive) continue;
            // balas del jugador atraviesan las defensas según requisito
        }

        for (auto& enemy : enemies.enemies) {
            if (!enemy.alive) continue;

            // Verificar colisión entre bala del jugador y enemigo
            if (RectCollision(bullet.rect, enemy.rect)) {
                // ¡Impacto!

                // Crear explosión de partículas en la posición del enemigo
                float explosionX = enemy.rect.x + enemy.rect.w / 2;
                float explosionY = enemy.rect.y + enemy.rect.h / 2;
                particles.CreateExplosion(explosionX, explosionY, 12);

                // Reproducir sonido de explosión del enemigo
                if (audioManager) {
                    audioManager->PlaySoundManager("enemy_explosion", 0.8f);
                }

                bullet.active = false;  // Destruir bala
                // Aplicar daño al enemigo y sólo ejecutar la lógica de muerte si realmente muere
                enemy.TakeDamage(1);

                if (enemy.IsAlive()) {
                    // Enemigo aún vivo tras el impacto
                    bulletConsumed = true;
                    break;
                }

                // Notificar al juego de la muerte (reglas de drop por nivel)
                // NOTE: la lógica de spawn por regla se consultará más abajo y devolverá
                // un booleano indicando si ya generó un power-up.

                // Si el enemigo era splitter, spawnear dos básicos en posiciones libres cercanas
                if (enemy.isSplitter || enemy.type == EnemyType::Splitter) {
                    // Determinar HP y velocidad para los hijos
                    int childHp = enemy.maxHealth / 2;
                    if (childHp < 1) childHp = 1;
                    float childSpeed = enemy.speed * 1.5f;

                    // Intentar colocar a la izquierda y derecha evitando solapamientos
                    float leftX = enemy.rect.x - enemy.rect.w - 8.0f;
                    float rightX = enemy.rect.x + enemy.rect.w + 8.0f;
                    float childY = enemy.rect.y;

                    auto isFree = [&](float x, float y) {
                        SDL_FRect r = { x, y, enemy.rect.w, enemy.rect.h };
                        for (auto& other : enemies.enemies) {
                            if (!other.alive) continue;
                            if (r.x < other.rect.x + other.rect.w && r.x + r.w > other.rect.x && r.y < other.rect.y + other.rect.h && r.y + r.h > other.rect.y)
                                return false;
                        }
                        return true;
                    };

                    // Ajustar límites de pantalla
                    const float screenW = 800.0f;
                    if (leftX < 0.0f) leftX = 8.0f;
                    if (rightX + enemy.rect.w > screenW) rightX = screenW - enemy.rect.w - 8.0f;

                    // Crear hijos si hay hueco, si no, buscar pequeñas correcciones
                    if (isFree(leftX, childY)) {
                        enemies.enemies.emplace_back(leftX, childY, childHp, EnemyColor(255,0,0,255), EnemyType::Basic, childSpeed, enemy.damage, MovePattern::Straight);
                    } else {
                        // buscar desplazamiento hacia la izquierda
                        for (int off = 16; off <= 160; off += 16) {
                            if (leftX - off >= 0 && isFree(leftX - off, childY)) {
                                enemies.enemies.emplace_back(leftX - off, childY, childHp, EnemyColor(255,0,0,255), EnemyType::Basic, childSpeed, enemy.damage, MovePattern::Straight);
                                break;
                            }
                        }
                    }

                    if (isFree(rightX, childY)) {
                        enemies.enemies.emplace_back(rightX, childY, childHp, EnemyColor(255,0,0,255), EnemyType::Basic, childSpeed, enemy.damage, MovePattern::Straight);
                    } else {
                        // buscar desplazamiento hacia la derecha
                        for (int off = 16; off <= 160; off += 16) {
                            if (rightX + off + enemy.rect.w <= screenW && isFree(rightX + off, childY)) {
                                enemies.enemies.emplace_back(rightX + off, childY, childHp, EnemyColor(255,0,0,255), EnemyType::Basic, childSpeed, enemy.damage, MovePattern::Straight);
                                break;
                            }
                        }
                    }
                }

                // Posibilidad de dropear powerup al morir (10%) o forzar en modo test
                // Pero si Game::OnEnemyKilled() ya spawnó un powerup (regla de nivel 1), saltar esta lógica.
                // Pasar la posición del enemigo para que el powerup forzado aparezca ahí
                bool spawnedByRule = game.OnEnemyKilled(enemy.rect.x + enemy.rect.w/2, enemy.rect.y + enemy.rect.h/2);
                if (spawnedByRule) {
                    // Ya se ha generado un powerup por la regla de nivel, no generar más
                } else {
                    bool forced = game.IsPowerupTestMode();
                    int baseChance = forced ? 100 : 8; // probabilidad base
                    // Darle un pequeño bonus en Nivel 2 (índice 1) para equilibrar drops
                    if (!forced && game.GetCurrentLevel() == 1) baseChance = 12;
                    if (rand() % 100 < baseChance) {
                        // Elegir tipo aleatorio de powerup
                        static int testIndex = 0;
                        PowerUp::Type chosen = PowerUp::Type::RestoreDefense;
                        if (forced) {
                            // En modo test, ciclar por tipos para poder probarlos todos
                            int idx = testIndex++;
                            switch (idx % 6) {
                                case 0: chosen = PowerUp::Type::RestoreDefense; break;
                                case 1: chosen = PowerUp::Type::BulletTime; break;
                                case 2: chosen = PowerUp::Type::ExtraLife; break;
                                case 3: chosen = PowerUp::Type::HomingMissiles; break;
                                case 4: chosen = PowerUp::Type::Shield; break;
                                case 5: chosen = PowerUp::Type::ContinueFire; break;
                            }
                        } else {
                            int r = rand() % 6;
                            switch (r) {
                                case 0: chosen = PowerUp::Type::RestoreDefense; break;
                                case 1: chosen = PowerUp::Type::BulletTime; break;
                                case 2: chosen = PowerUp::Type::ExtraLife; break;
                                case 3: chosen = PowerUp::Type::HomingMissiles; break;
                                case 4: chosen = PowerUp::Type::Shield; break;
                                case 5: chosen = PowerUp::Type::ContinueFire; break;
                            }
                        }

                        // Spawn powerup en la posición del enemigo
                        PowerUp pu(enemy.rect.x + enemy.rect.w/2 - 9.0f, enemy.rect.y + enemy.rect.h/2, chosen);
                        game.SpawnPowerUp(pu);
                        std::cout << "[CollisionManager] PowerUp spawned (" << (int)chosen << ") at " << pu.rect.x << "," << pu.rect.y << std::endl;
                    }
                }

                // Sumar puntos
                game.AddScore(10);

                std::cout << "¡Enemigo destruido con explosión! +10 puntos" << std::endl;
                bulletConsumed = true;
                break; // La bala ya impactó, no necesita seguir verificando
            }
        }
        if (bulletConsumed) continue;
    }

    // Colisiones: balas enemigas con jugador y bloques defensivos
    for (auto& bullet : enemyBullets) {
        if (!bullet.active) continue;

        bool bulletHandled = false;
        // Primero verificar colisión con bloques defensivos
        for (auto& block : blocks) {
            if (!block.alive) continue;
            if (RectCollision(bullet.rect, block.rect)) {
                // Bala enemiga golpea bloque defensivo
                block.TakeBulletHit();
                bullet.active = false;
                bulletHandled = true;
                // Partículas pequeñas
                particles.CreateExplosion(bullet.rect.x + bullet.rect.w/2, bullet.rect.y + bullet.rect.h/2, 6);
                break;
            }
        }
        if (bulletHandled) continue;

        // Verificar colisión entre bala enemiga y jugador
        if (RectCollision(bullet.rect, player.rect)) {
            // ¡El jugador fue impactado!

            // Crear explosión de partículas en la posición del jugador
            float explosionX = player.rect.x + player.rect.w / 2;
            float explosionY = player.rect.y + player.rect.h / 2;
            particles.CreateExplosion(explosionX, explosionY, 8);

            // Reproducir sonido de muerte del jugador
            if (audioManager) {
                audioManager->PlaySoundManager("player_death", 0.9f);
            }

            bullet.active = false;  // Destruir bala

            // El jugador pierde una vida
            game.LoseLife();

            std::cout << "¡Jugador impactado! Vidas restantes: " << game.GetLives() << std::endl;
            continue; // Solo una bala puede impactar por frame
        }
    }

    // Colisiones: enemigos tocando bloques defensivos (destruyen bloque y enemigo)
    for (auto& enemy : enemies.enemies) {
        if (!enemy.alive) continue;
        for (auto& block : blocks) {
            if (!block.alive) continue;
            if (RectCollision(enemy.rect, block.rect)) {
                block.DestroyByEnemy();
                enemy.alive = false; // enemigo se destruye al tocar el bloque
                particles.CreateExplosion(enemy.rect.x + enemy.rect.w/2, enemy.rect.y + enemy.rect.h/2, 10);
                if (audioManager) audioManager->PlaySoundManager("enemy_explosion", 0.8f);
            }
        }
    }

    // Detectar colisiones powerups con jugador
    auto& powerups = game.GetPowerUps();
    for (auto& pu : powerups) {
        if (!pu.active) continue;
        if (RectCollision(pu.rect, player.rect)) {
            pu.active = false;
            switch (pu.type) {
                case PowerUp::Type::RestoreDefense: {
                    // Restaurar hasta 3 piezas de defensa
                    int restored = 0;
                    for (auto& block : blocks) {
                        if (restored >= 3) break;
                        if (!block.alive) {
                            block.alive = true;
                            block.hp = 3;
                            block.rect = block.originalRect;
                            restored++;
                        }
                    }
                    std::cout << "[CollisionManager] PowerUp collected: restored " << restored << " defense pieces" << std::endl;
                    break;
                }
                case PowerUp::Type::BulletTime: {
                    // Activar bullet time a través de la API de Game
                    game.ActivateBulletTime(3.0f);
                    std::cout << "[CollisionManager] Bullet Time requested for 3s" << std::endl;
                     break;
                 }
                 case PowerUp::Type::ExtraLife: {
                    game.AddLives(1);
                    std::cout << "[CollisionManager] Extra life requested" << std::endl;
                     break;
                 }
                     case PowerUp::Type::HomingMissiles: {
                    game.AddHomingMissiles(3);
                    std::cout << "[CollisionManager] Homing missiles requested" << std::endl;
                     break;
                 }
                      case PowerUp::Type::ContinueFire: {
                          // Activar ContinueFire: reducir cadencia por 3s
                          game.ActivateContinueFire(3.0f);
                          std::cout << "[CollisionManager] ContinueFire requested (3s)" << std::endl;
                          break;
                      }
                 case PowerUp::Type::Shield: {
                    game.ActivateShield(3, 2.0f);
                    std::cout << "[CollisionManager] Shield requested: 3 hits, 2s" << std::endl;
                     break;
                 }
             }
         }
     }

    // Detectar enemigos que hayan cruzado la parte inferior de la ventana
    for (auto& enemy : enemies.enemies) {
        if (!enemy.alive) continue;
        const float screenH = 600.0f;
        if (enemy.rect.y + enemy.rect.h >= screenH) {
            // El enemigo ha escapado
            enemy.alive = false; // eliminar enemigo

            // Crear explosión en su posición
            particles.CreateExplosion(enemy.rect.x + enemy.rect.w/2, enemy.rect.y + enemy.rect.h/2, 12);
            if (audioManager) audioManager->PlaySoundManager("enemy_explosion", 0.9f);

            // Restar una vida al jugador
            game.LoseLife();
            std::cout << "[CollisionManager] Enemy escaped bottom. Player loses a life. Lives left: " << game.GetLives() << std::endl;
        }
    }
}
