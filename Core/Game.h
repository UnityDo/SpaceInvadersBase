#pragma once
#include "Player.h"
#include "EnemyManager.h"
#include "Bullet.h"
#include "Renderer.h"
#include "InputManager.h"
#include "CollisionManager.h"
#include "ParticleSystem.h"
#include "TextRenderer.h"
#include "AudioManagerBeep.h"
#include "AudioManagerMiniaudio.h"
#include <vector>

class Game {
public:
    Game();
    ~Game();
    bool Init();
    void Run();
    void Shutdown();
    
    // Sistema de puntuación
    void AddScore(int points);
    int GetScore() const;
    
    // Sistema de vidas
    void LoseLife();
    int GetLives() const;
    bool IsGameOver() const;
    
    // Sistema de victoria
    void CheckForVictory();
    bool IsGameWon() const;
    
private:
    bool running;
    Player* player;
    EnemyManager* enemyManager;
    Renderer* renderer;
    InputManager* inputManager;
    CollisionManager* collisionManager;
    ParticleSystem* particleSystem;
    TextRenderer* textRenderer;
    AudioManagerMiniaudio* audioManager;
    std::vector<Bullet> bullets;
    std::vector<Bullet> enemyBullets;
    
    // Estado del juego
    int score;
    int lives;
    bool gameOver;
    bool gameWon;
    float enemyShootTimer;
};
