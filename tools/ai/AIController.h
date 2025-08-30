#pragma once
#include "../../Core/IPlayerController.h"
#include <random>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include "../../Core/Raycast.h"

namespace tools { namespace ai {

class AIController : public IPlayerController {
public:
    AIController(unsigned int seed = 0) {
        if (seed == 0) seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
        rng.seed(seed);
    // Load tunable params from environment (used by Optuna tuning script)
    const char* ew = std::getenv("AI_ENEMY_W");
    const char* eh = std::getenv("AI_ENEMY_H");
    const char* op = std::getenv("AI_OCCLUSION_PENALTY");
    if (ew) enemyW_env = static_cast<float>(std::atof(ew));
    if (eh) enemyH_env = static_cast<float>(std::atof(eh));
    if (op) occlusionPenalty_env = static_cast<float>(std::atof(op));
    }
    // Ensure an out-of-line destructor so the vtable is emitted in the .cpp
    virtual ~AIController();
    void Update(float dt) override;
    bool WantsMoveLeft() const override { return moveLeft; }
    bool WantsMoveRight() const override { return moveRight; }
    bool WantsFire() const override { return fire; }
    bool WantsUseShield() const override { return false; }
    void Observe(const WorldObservation& obs) override {
        lastObs = obs;
        playerX = obs.playerX;
    }
    // Simple observation injection (optional)
    void ObservePlayerX(float x) { playerX = x; }
private:
    std::mt19937 rng;
    float playerX = 0.0f;
    WorldObservation lastObs;
    // tunable params (defaults match previous constants)
    float enemyW_env = 44.0f;
    float enemyH_env = 30.0f;
    float occlusionPenalty_env = 10.0f;
    // Simple policy state
    bool moveLeft = false;
    bool moveRight = false;
    bool fire = false;
};

}} // namespace
