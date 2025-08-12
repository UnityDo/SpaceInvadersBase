
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
            // Mover horizontalmente más distancia
            for (auto& e : enemies) {
                if (e.alive) {
                    e.rect.x += 35 * direction; // Movimiento por pasos más grandes
                }
            }
        }
    }
    
    for (auto& e : enemies) e.Update(dt);
}

void EnemyManager::FireRandomBullet(std::vector<Bullet>& enemyBullets) {
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
}
