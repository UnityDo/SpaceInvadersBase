#include "CollisionManager.h"
#include "Game.h"
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
    // Colisiones: balas del jugador con enemigos
    for (auto& bullet : playerBullets) {
        if (!bullet.active) continue;
        
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
                enemy.alive = false;    // Destruir enemigo
                
                // Sumar puntos
                game.AddScore(10);
                
                std::cout << "¡Enemigo destruido con explosión! +10 puntos" << std::endl;
                break; // La bala ya impactó, no necesita seguir verificando
            }
        }
    }
    
    // Colisiones: balas enemigas con jugador
    for (auto& bullet : enemyBullets) {
        if (!bullet.active) continue;
        
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
            break; // Solo una bala puede impactar por frame
        }
    }
}
