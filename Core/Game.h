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
#include "PowerUp.h"
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

    // PowerUp API
    void SpawnPowerUp(const PowerUp& pu);
    std::vector<PowerUp>& GetPowerUps();

    // API pública para efectos de powerups
    void ActivateBulletTime(float seconds);
    void ActivateContinueFire(float seconds);
    void AddLives(int n);
    void AddHomingMissiles(int n);
    void ActivateShield(int hp, float duration);

    // Historial de partidas
    void SaveGameHistoryEntry();

    // Testing helpers
    void SetPowerupTestMode(bool enable);
    bool IsPowerupTestMode() const;
    // Expose current level
    int GetCurrentLevel() const;
    // Fire cooldown state (seconds between shots)
    float GetPlayerFireCooldown() const;
    // Called when an enemy is killed (for level-specific drop rules)
    // Returns true if the call spawned a forced power-up (so callers can skip their own spawn logic)
    // spawnX/spawnY optional: position of the killed enemy to spawn forced powerup there
    bool OnEnemyKilled(float spawnX = -1.0f, float spawnY = -1.0f);

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
    std::vector<PowerUp> powerUps;
    // Estados globales de powerups
    float bulletTimeTimer = 0.0f; // tiempo restante de bullet-time
    int homingMissilesCount = 0;  // cuántos misiles homing restantes
    bool shieldActive = false;
    float shieldTimer = 0.0f;    // duración restante del escudo
    int shieldHp = 0;            // impacto que puede absorber
    // Control de cadencia de disparo del jugador
    float playerFireCooldown = 0.25f; // segundos entre disparos por defecto
    float playerFireTimer = 0.0f; // temporizador para controlar disparo continuo
    // ContinueFire powerup reduce el cooldown a continuación
    float continueFireCooldown = 0.08f; // cooldown reducido durante powerup
    float continueFireTimer = 0.0f; // tiempo restante de ContinueFire
     
     // Estado del juego
     int score;
     int lives;
     bool gameOver;
     bool gameWon;
     float enemyShootTimer;
        // Avance de niveles
        int currentLevel = 0;
        // Número total de niveles (25)
        int maxLevels = 25;
        bool levelTransition = false;
        bool finalVictory = false;
        void NextLevel();
        void ShowLevelTransition();

        // Tiempos de partida
        double startTime = 0.0;
        double elapsedTime = 0.0;
    // Test mode: si true, todos los enemigos sueltan powerups para testing
    bool powerupTestMode = false;
    // Conteo de muertes desde el inicio del nivel (para reglas como Level 1 -> drop tras 3 kills)
    int killsSinceLevelStart = 0;
};
