#include "Game.h"
#include <SDL3/SDL.h>

#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include "../libs/nlohmann/json.hpp"
using json = nlohmann::json;

#include "AudioManagerMiniaudio.h"

// Evitar conflicto con macro DrawText de Windows
#ifdef DrawText
#undef DrawText
#endif

bool g_running = true;

Game::Game() : running(false), player(nullptr), enemyManager(nullptr), renderer(nullptr), inputManager(nullptr), collisionManager(nullptr), particleSystem(nullptr), textRenderer(nullptr), audioManager(nullptr), score(0), lives(3), gameOver(false), gameWon(false), enemyShootTimer(0.0f) {}

Game::~Game() { Shutdown(); }

bool Game::Init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("No se pudo inicializar SDL: %s", SDL_GetError());
        return false;
    }
    renderer = new Renderer();
    if (!renderer->Init()) {
        SDL_Log("No se pudo crear ventana o renderer");
        return false;
    }
    // Iniciar cronómetro de la partida
    startTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
    player = new Player();
    enemyManager = new EnemyManager();
    inputManager = new InputManager();

    // Usar sistema de audio miniaudio
    audioManager = new AudioManagerMiniaudio();
    if (!audioManager->Initialize()) {
        SDL_Log("Error al inicializar el sistema de audio (miniaudio)");
        return false;
    }
    // Cargar sonidos desde assets
    audioManager->LoadSound("player_shoot", "assets/player_shoot.wav");
    audioManager->LoadSound("enemy_explosion", "assets/enemy_explosion.wav");
    audioManager->LoadSound("player_death", "assets/player_death.wav");
    
    collisionManager = new CollisionManager(audioManager);
    particleSystem = new ParticleSystem();
    textRenderer = new TextRenderer();
    
    if (!textRenderer->Init()) {
        std::cout << "Warning: No se pudo inicializar TextRenderer" << std::endl;
    }
    
    running = true;
    g_running = true;
    return true;
}

// Dibuja un círculo relleno en SDL
void DrawCircle(SDL_Renderer* rend, int cx, int cy, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, color.a);
    for (int w = -radius; w <= radius; w++) {
        for (int h = -radius; h <= radius; h++) {
            if (w * w + h * h <= radius * radius) {
                SDL_RenderPoint(rend, cx + w, cy + h);
            }
        }
    }
}

void Game::Run() {
    SDL_Renderer* rend = renderer->GetSDLRenderer();
    SDL_Event e;
    while (running) {
        // Limpiar estado de input del frame anterior
        inputManager->Update();
        
        // Gestionar eventos de ventana Y pasarlos al InputManager
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) running = false;
                // Si estamos en transición de nivel, cualquier tecla avanza
                if (levelTransition && e.type == SDL_EVENT_KEY_DOWN) {
                    levelTransition = false;
                    NextLevel();
                }
            // Pasar el evento al InputManager para procesamiento
            inputManager->HandleEvent(e);
        }
        
        // dt real del frame (puedes sustituir por un timer real si tienes uno)
        const float realDt = 0.016f;
        // timeScale reduzca todo excepto el jugador
        float timeScale = (bulletTimeTimer > 0.0f) ? 0.35f : 1.0f;
        float scaledDt = realDt * timeScale;

        if (inputManager->IsLeftPressed()) {
            player->Move(-1.0f, realDt);
        }
        if (inputManager->IsRightPressed()) {
            player->Move(1.0f, realDt);
        }
        if (inputManager->IsFirePressed()) {
            // Crear nueva bala en la posición del jugador
            float bulletX = player->rect.x + player->rect.w / 2 - 2.5f; // Centrar la bala
            float bulletY = player->rect.y - 10; // Arriba del jugador
            // Si tenemos misiles homing, crear bala homing y consumir contador
            bool spawnHoming = false;
            if (homingMissilesCount > 0) {
                spawnHoming = true;
                homingMissilesCount -= 1;
            }

            if (spawnHoming) {
                // Intentar apuntar al enemigo más cercano
                float vx = 0.0f;
                float vy = -300.0f;
                float bx = bulletX + 2.5f;
                float by = bulletY + 7.5f;
                float bestDist = 1e9f;
                // Buscar enemigo más cercano
                for (const auto& en : enemyManager->enemies) {
                    if (!en.alive) continue;
                    float ex = en.rect.x + en.rect.w / 2;
                    float ey = en.rect.y + en.rect.h / 2;
                    float dx = ex - bx;
                    float dy = ey - by;
                    float dist = sqrtf(dx*dx + dy*dy);
                    if (dist < bestDist) {
                        bestDist = dist;
                        // estimar tiempo para alcanzar verticalmente
                        float t = fabsf(dy) / fabsf(vy);
                        if (t < 0.001f) t = 0.5f;
                        vx = dx / t;
                    }
                }
                bullets.emplace_back(bulletX, bulletY, vy, vx, true);
            } else {
                bullets.emplace_back(bulletX, bulletY, -300.0f, 0.0f, false); // Velocidad hacia arriba
            }
             
             // Reproducir sonido de disparo
             audioManager->PlaySoundManager("player_shoot", 0.7f);

             if (!levelTransition && !finalVictory) {
                player->Update(realDt);
             }
         }

        player->Update(realDt);
        
        // Actualizar balas del jugador
        for (auto& bullet : bullets) {
            // Las balas del jugador no se ven afectadas por bullet-time
            bullet.Update(realDt);
         }
         
         // Actualizar balas enemigas
        for (auto& bullet : enemyBullets) {
            // Las balas enemigas se ralentizan durante bullet-time
            bullet.Update(scaledDt);
        }

         // Actualizar powerups
        for (auto& pu : powerUps) pu.Update(scaledDt);
         
         // Eliminar balas inactivas
         bullets.erase(std::remove_if(bullets.begin(), bullets.end(), 
             [](const Bullet& b) { return !b.active; }), bullets.end());
         enemyBullets.erase(std::remove_if(enemyBullets.begin(), enemyBullets.end(), 
             [](const Bullet& b) { return !b.active; }), enemyBullets.end());
            if (!levelTransition && !finalVictory) {
                // Los enemigos se mueven con timeScale
                enemyManager->Update(scaledDt);
            }
         
        // Disparos enemigos (se ralentizan con bullet-time)
        enemyShootTimer += scaledDt;
         if (enemyShootTimer >= 1.5f) { // Disparar cada 1.5 segundos
             enemyManager->FireRandomBullet(enemyBullets);
             enemyShootTimer = 0.0f;
         }
         
         // Actualizar sistema de partículas
        particleSystem->Update(scaledDt);
         
         // Verificar colisiones
         collisionManager->CheckCollisions(*player, *enemyManager, bullets, enemyBullets, *particleSystem, *this);
            if (!levelTransition && !finalVictory) {
                CheckForVictory();
            }

        // Actualizar timers de powerups globales con tiempo real
        if (bulletTimeTimer > 0.0f) {
            bulletTimeTimer -= realDt;
            if (bulletTimeTimer < 0.0f) bulletTimeTimer = 0.0f;
        }

         renderer->Clear();
        
        if (gameOver) {
            // Pantalla de Game Over
            if (textRenderer) {
                SDL_Color red = {255, 0, 0, 255};
                SDL_Color white = {255, 255, 255, 255};
                textRenderer->RenderText(rend, "GAME OVER", 300, 250, red);
                std::string finalScore = "Final Score: " + std::to_string(score);
                textRenderer->RenderText(rend, finalScore, 280, 300, white);
                textRenderer->RenderText(rend, "Press ESC to exit", 280, 350, white);
            }
        } else if (finalVictory) {
            // Pantalla de victoria final
            if (textRenderer) {
                SDL_Color gold = {255, 215, 0, 255};
                SDL_Color white = {255, 255, 255, 255};
                textRenderer->RenderText(rend, "¡VICTORIA FINAL!", 270, 200, gold);
                textRenderer->RenderText(rend, "Has superado todos los niveles", 200, 250, gold);
                std::string finalScore = "Score final: " + std::to_string(score);
                textRenderer->RenderText(rend, finalScore, 280, 300, white);
                textRenderer->RenderText(rend, "Gracias por jugar", 290, 350, white);
                textRenderer->RenderText(rend, "Press ESC to exit", 280, 400, white);
            }
        } else if (levelTransition) {
            // Pantalla de transición de nivel
            ShowLevelTransition();
        } else {
            // Juego normal
            // Dibujar jugador como círculo
            SDL_Color green = {0,255,0,255};
            DrawCircle(rend, (int)(player->rect.x + player->rect.w/2), (int)(player->rect.y + player->rect.h/2), (int)(player->rect.w/2), green);
            
            // Dibujar balas del jugador
            for (auto& bullet : bullets) {
                bullet.Render(rend);
            }
            
            // Dibujar balas enemigas (rojas)
            for (auto& bullet : enemyBullets) {
                SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
                SDL_RenderFillRect(rend, &bullet.rect);
            }
            
            // Usar el método de EnemyManager para renderizar enemigos y defensas
            enemyManager->Render(rend);

            // Dibujar partículas
            particleSystem->Render(rend);
            
            // Dibujar UI - Score y vidas
            if (textRenderer) {
                std::string scoreText = "Score: " + std::to_string(score);
                std::string livesText = "Lives: " + std::to_string(lives);
                SDL_Color white = {255, 255, 255, 255};
                textRenderer->RenderText(rend, scoreText, 20, 20, white);
                textRenderer->RenderText(rend, livesText, 20, 50, white);
                // Mostrar nivel actual
                std::string levelText = "Level: " + std::to_string(currentLevel + 1) + "/25";
                textRenderer->RenderText(rend, levelText, 20, 80, white);
            }

            // Dibujar powerups
            for (auto& pu : powerUps) pu.Render(rend);
        }
        renderer->Present();
        SDL_Delay(16);
    }
}

void Game::Shutdown() {
    delete player;
    delete enemyManager;
    delete renderer;
    delete inputManager;
    delete collisionManager;
    delete particleSystem;
    delete textRenderer;
    delete audioManager;
    SDL_Quit();
}

void Game::AddScore(int points) {
    score += points;
}

int Game::GetScore() const {
    return score;
}

void Game::LoseLife() {
    lives--;
    if (lives <= 0) {
        gameOver = true;
        // Registrar en historial
        elapsedTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count() - startTime;
        SaveGameHistoryEntry();
    }
}

int Game::GetLives() const {
    return lives;
}

bool Game::IsGameOver() const {
    return gameOver;
}

void Game::CheckForVictory() {
    if (gameOver || levelTransition || finalVictory) {
        return; // Si ya el juego terminó o está en transición, no verificar
    }
    // Contar enemigos vivos
    int aliveEnemies = 0;
    for (const auto& enemy : enemyManager->enemies) {
        if (enemy.alive) {
            aliveEnemies++;
        }
    }
    // Si no hay enemigos vivos, activar transición o victoria final
    if (aliveEnemies == 0) {
        if (currentLevel >= 24) { // 0-indexed, nivel 25
            finalVictory = true;
            // Registrar victoria en historial
            elapsedTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count() - startTime;
            SaveGameHistoryEntry();
            std::cout << "¡VICTORIA FINAL! Has superado todos los niveles. Score final: " << score << std::endl;
        } else {
            levelTransition = true;
            std::cout << "Nivel superado: " << (currentLevel+1) << ". Pulsa una tecla para continuar." << std::endl;
        }
        gameWon = false;
        gameOver = false;
    }
}

void Game::SaveGameHistoryEntry() {
    try {
        json entry;
        entry["duration_seconds"] = elapsedTime;
        entry["max_level"] = currentLevel + 1; // humano-friendly
        entry["lives"] = lives;
        entry["score"] = score;

        // Leer archivo existente
        std::string path = "Data/games_history.json";
        json root;
        std::ifstream in(path);
        if (in.good()) {
            in >> root;
            in.close();
            if (!root.is_array()) root = json::array();
        } else {
            root = json::array();
        }

        // Añadir entrada y escribir
        root.push_back(entry);
        std::ofstream out(path);
        out << root.dump(2);
        out.close();

        std::cout << "[Game] Game history entry saved." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Game] Failed to save game history: " << e.what() << std::endl;
    }
}

bool Game::IsGameWon() const {
    return gameWon;
}

void Game::NextLevel() {
    currentLevel++;
    if (currentLevel >= maxLevels) {
        finalVictory = true;
        return;
    }

    // No reactivar la pantalla de transición aquí: ya se ha pulsado la tecla
    levelTransition = false;

    // Limpiar power-ups al cambiar de nivel para que no persistan
    powerUps.clear();
    
    // Cargar el siguiente nivel
    enemyManager->LoadLevel(currentLevel);
     // Resetear balas y estados
     bullets.clear();
     enemyBullets.clear();
     gameWon = false;
     gameOver = false;
     player->rect.x = 350; // Centrar jugador
     player->rect.y = 550;
}

void Game::ShowLevelTransition() {
    if (textRenderer) {
        SDL_Color yellow = {255, 255, 0, 255};
        SDL_Color white = {255, 255, 255, 255};
        std::string msg = "Nivel superado: " + std::to_string(currentLevel+1);
        textRenderer->RenderText(renderer->GetSDLRenderer(), msg, 250, 250, yellow);
        textRenderer->RenderText(renderer->GetSDLRenderer(), "Pulsa cualquier tecla para continuar", 180, 300, white);
    }
}

void Game::SpawnPowerUp(const PowerUp& pu) {
    powerUps.push_back(pu);
}

std::vector<PowerUp>& Game::GetPowerUps() {
    return powerUps;
}

// API de powerups
void Game::ActivateBulletTime(float seconds) {
    bulletTimeTimer = std::max(bulletTimeTimer, seconds);
    std::cout << "[Game] BulletTime activated: " << bulletTimeTimer << "s" << std::endl;
}

void Game::AddLives(int n) {
    lives += n;
    std::cout << "[Game] Lives increased: " << lives << std::endl;
}

void Game::AddHomingMissiles(int n) {
    homingMissilesCount += n;
    std::cout << "[Game] Homing missiles now: " << homingMissilesCount << std::endl;
}

void Game::ActivateShield(int hp, float duration) {
    shieldActive = true;
    shieldHp = hp;
    shieldTimer = duration;
    // sincronizar con player si existe
    if (player) {
        player->shieldActive = true;
        player->shieldHp = hp;
        player->shieldTimer = duration;
        player->shieldAlpha = 0.15f;
    }
    std::cout << "[Game] Shield activated: hp=" << hp << " duration=" << duration << std::endl;
}
