#pragma once
#include "Entity.h"
#include "IEnemy.h" 

struct EnemyColor {
    Uint8 r, g, b, a;
    EnemyColor(Uint8 r_, Uint8 g_, Uint8 b_, Uint8 a_ = 255) : r(r_), g(g_), b(b_), a(a_) {}
};


enum class EnemyType {
    Basic,
    Fast,
    Tank,
    Boss,
    Sniper,
    Splitter
};

enum class MovePattern {
    Straight,
    ZigZag,
    Diagonal,
    Dive,
    DescendStopShoot,
    Circle,
    Stationary,
    Scatter,
    None
};

class Enemy : public Entity, public IEnemy {
public:
    Enemy(float x, float y, int hp = 1, EnemyColor color = EnemyColor(255,0,0,255),
          EnemyType type = EnemyType::Basic, float speed = 1.0f, int damage = 1, MovePattern pattern = MovePattern::Straight);
    void Update(float dt) override;
    void Render(SDL_Renderer* renderer) override;
        void TakeDamage(int amount) override; // Declaration only, implementation in Enemy.cpp
    bool IsAlive() const override { return alive; }

    bool alive = true;
    int health = 1;
    int maxHealth = 1;
    int damage = 1;
    float speed = 1.0f;
    EnemyColor color;
    EnemyType type = EnemyType::Basic;
    MovePattern pattern = MovePattern::Straight;
    bool isSplitter = false;
    bool isBoss = false;
    bool isSniper = false;
    float patternTimer = 0.0f;
    int phase = 0; // Para bosses

    // Comportamiento especial para bosses
    enum class BossAction { None, Maneuver, Teleport, TripleShot };
    BossAction bossAction = BossAction::None;
    float actionTimer = 0.0f;      // tiempo restante de la acción
    float actionDuration = 0.0f;   // duración planificada
    float decisionInterval = 3.0f; // cada cuantos segundos decide una acción
    float decisionTimer = 0.0f;

    // Teleport pending
    float pendingTeleportX = -1.0f;
    bool wantsTeleport = false;
};
