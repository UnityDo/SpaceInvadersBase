#include "AIController.h"
#include <chrono>

namespace tools { namespace ai {

AIController::AIController(unsigned int seed) {
    if (seed == 0) seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
    rng.seed(seed);
}

void AIController::Update(float dt) {
    // Política muy simple: disparar cuando haya enemigos, moverse aleatoriamente
    std::uniform_real_distribution<float> d(0.0f, 1.0f);
    float r = d(rng);
    fire = (enemiesCount > 0) ? (r > 0.3f) : false;
    moveLeft = r < 0.45f;
    moveRight = r > 0.55f;
}

bool AIController::WantsMoveLeft() const { return moveLeft; }
bool AIController::WantsMoveRight() const { return moveRight; }
bool AIController::WantsFire() const { return fire; }
bool AIController::WantsUseShield() const { return false; }

}} // namespace
