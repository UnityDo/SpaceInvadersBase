#include "Game.h"
#include <SDL3/SDL.h>

#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

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
            // Pasar el evento al InputManager para procesamiento
            inputManager->HandleEvent(e);
        }
        
        if (inputManager->IsLeftPressed()) {
            player->Move(-5.0f);
        }
        if (inputManager->IsRightPressed()) {
            player->Move(5.0f);
        }
        if (inputManager->IsFirePressed()) {
            // Crear nueva bala en la posición del jugador
            float bulletX = player->rect.x + player->rect.w / 2 - 2.5f; // Centrar la bala
            float bulletY = player->rect.y - 10; // Arriba del jugador
            bullets.emplace_back(bulletX, bulletY, -300.0f); // Velocidad hacia arriba
            
            // Reproducir sonido de disparo
            audioManager->PlaySoundManager("player_shoot", 0.7f);

        }

        player->Update(0.016f);
        
        // Actualizar balas del jugador
        for (auto& bullet : bullets) {
            bullet.Update(0.016f);
        }
        
        // Actualizar balas enemigas
        for (auto& bullet : enemyBullets) {
            bullet.Update(0.016f);
        }
        
        // Eliminar balas inactivas
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(), 
            [](const Bullet& b) { return !b.active; }), bullets.end());
        enemyBullets.erase(std::remove_if(enemyBullets.begin(), enemyBullets.end(), 
            [](const Bullet& b) { return !b.active; }), enemyBullets.end());
        
        enemyManager->Update(0.016f);
        
        // Disparos enemigos
        enemyShootTimer += 0.016f;
        if (enemyShootTimer >= 1.5f) { // Disparar cada 1.5 segundos
            enemyManager->FireRandomBullet(enemyBullets);
            enemyShootTimer = 0.0f;
        }
        
        // Actualizar sistema de partículas
        particleSystem->Update(0.016f);
        
        // Verificar colisiones
        collisionManager->CheckCollisions(*player, *enemyManager, bullets, enemyBullets, *particleSystem, *this);

        // Verificar si el jugador ganó (todos los enemigos eliminados)
        CheckForVictory();

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
        } else if (gameWon) {
            // Pantalla de Victoria
            if (textRenderer) {
                SDL_Color gold = {255, 215, 0, 255};
                SDL_Color green = {0, 255, 0, 255};
                SDL_Color white = {255, 255, 255, 255};
                textRenderer->RenderText(rend, "¡VICTORIA!", 320, 200, gold);
                textRenderer->RenderText(rend, "Todos los enemigos eliminados", 220, 250, green);
                std::string finalScore = "Final Score: " + std::to_string(score);
                textRenderer->RenderText(rend, finalScore, 280, 300, white);
                textRenderer->RenderText(rend, "Gracias por jugar", 290, 350, white);
                textRenderer->RenderText(rend, "Press ESC to exit", 280, 400, white);
            }
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
            
            // Dibujar enemigos como cuadrados
            for (auto& e : enemyManager->enemies) {
                if (e.alive) {
                    SDL_SetRenderDrawColor(rend, 255,0,0,255);
                    SDL_FRect r = e.rect;
                    SDL_FRect rect = {r.x, r.y, r.w, r.h};
                    SDL_RenderFillRect(rend, &rect);
                }
            }
            
            // Dibujar partículas
            particleSystem->Render(rend);
            
            // Dibujar UI - Score y vidas
            if (textRenderer) {
                std::string scoreText = "Score: " + std::to_string(score);
                std::string livesText = "Lives: " + std::to_string(lives);
                SDL_Color white = {255, 255, 255, 255};
                textRenderer->RenderText(rend, scoreText, 20, 20, white);
                textRenderer->RenderText(rend, livesText, 20, 50, white);
            }
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
    }
}

int Game::GetLives() const {
    return lives;
}

bool Game::IsGameOver() const {
    return gameOver;
}

void Game::CheckForVictory() {
    if (gameOver || gameWon) {
        return; // Si ya el juego terminó, no verificar
    }
    
    // Contar enemigos vivos
    int aliveEnemies = 0;
    for (const auto& enemy : enemyManager->enemies) {
        if (enemy.alive) {
            aliveEnemies++;
        }
    }
    
    // Si no hay enemigos vivos, el jugador ganó
    if (aliveEnemies == 0) {
        gameWon = true;
        std::cout << "¡VICTORIA! Todos los enemigos eliminados. Score final: " << score << std::endl;
    }
}

bool Game::IsGameWon() const {
    return gameWon;
}
