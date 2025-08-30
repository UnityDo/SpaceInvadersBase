#include "AIController.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include "../../Core/Raycast.h"
#include "../../Core/PowerUp.h"

namespace tools { namespace ai {

AIController::~AIController() {}

void AIController::Update(float dt) {
    // Política avanzada: 1º evasión de balas, 2º powerups críticos, 3º enemigos, 4º powerups normales
    moveLeft = moveRight = fire = false;

    // Parameters and quick references
    const float playerXF = playerX;
    const float playerYF = lastObs.playerY;
    const float interceptThresholdY = 480.0f; // urgency zone near bottom

    // 0) PRIORITY 1: BULLET EVASION - Check for incoming enemy bullets and evade
    for (const auto &bullet : lastObs.enemyBullets) {
        // Predict where the bullet will be when it reaches player Y level
        float timeToReachPlayer = (playerYF - bullet.y) / bullet.vy;
        if (timeToReachPlayer <= 0.0f || timeToReachPlayer > 3.0f) continue; // bullet moving away or too far
        
        // Predicted bullet X position when it reaches player level
        float predictedBulletX = bullet.x + bullet.vx * timeToReachPlayer;
        float dangerRadius = 25.0f; // collision avoidance radius
        
        // Check if bullet will hit player if player doesn't move
        if (std::abs(predictedBulletX - playerXF) < dangerRadius && timeToReachPlayer < 1.5f) {
            // Determine best evasion direction
            float leftSpace = playerXF - 50.0f; // space to move left (screen boundary consideration)
            float rightSpace = 750.0f - playerXF; // space to move right
            
            if (predictedBulletX > playerXF) {
                // Bullet coming from right, move left if possible
                if (leftSpace > 30.0f) moveLeft = true;
                else moveRight = true; // fallback: move right
            } else {
                // Bullet coming from left, move right if possible
                if (rightSpace > 30.0f) moveRight = true;
                else moveLeft = true; // fallback: move left
            }
            
            // Emergency evasion: fire while evading (might help clear path)
            fire = true;
            return; // HIGHEST PRIORITY: exit immediately after evasion command
        }
    }

    // 1) PRIORITY 2: Immediate urgent enemy response: if an enemy is very low and near X, intercept
    for (const auto &e : lastObs.enemies) {
        if (e.hp <= 0) continue;
        if (e.y > interceptThresholdY) {
            float dx = e.x - playerXF;
            if (std::abs(dx) < 140.0f) {
                if (dx < -6.0f) moveLeft = true;
                else if (dx > 6.0f) moveRight = true;
                fire = true;
                return; // immediate reaction
            }
        }
    }

    // Build enemy rects used for occlusion checks (ignore defense blocks)
    const float eW = enemyW_env, eH = enemyH_env;
    std::vector<SDL_FRect> enemyRects; enemyRects.reserve(lastObs.enemies.size());
    std::vector<int> enemyIdx; enemyIdx.reserve(lastObs.enemies.size());
    for (size_t i = 0; i < lastObs.enemies.size(); ++i) {
        const auto &e = lastObs.enemies[i];
        if (e.hp <= 0) continue;
        enemyIdx.push_back((int)i);
        enemyRects.push_back(SDL_FRect{ e.x - eW*0.5f, e.y - eH*0.5f, eW, eH });
    }

    // 2) PRIORITY 3: Evaluate power-ups: predict fall time and movement time, compute a score
    int bestPU = -1; float bestScore = 1e9f; std::vector<float> puTimes(lastObs.powerups.size(), 1e9f);
    for (size_t i = 0; i < lastObs.powerups.size(); ++i) {
        const auto &pu = lastObs.powerups[i];
        float dx = pu.x - playerXF;
        float dy = pu.y - playerYF; // vertical from player
        // assume falling speed approx 120 units/sec (empirical); if dy<0, it's above the player
        float timeToFall = std::abs(dy) / 120.0f;
        float timeToMove = std::abs(dx) / 300.0f; // player horizontal speed
        float totalTime = timeToFall + timeToMove;
        puTimes[i] = totalTime;

        // Check occlusion: raycast between player and powerup against enemy rects
        Core::Raycast::HitResult hit;
        SDL_FPoint p0{ playerXF, playerYF };
        SDL_FPoint p1{ pu.x, pu.y };
        int hitIdx = Core::Raycast::RaycastRects(p0, p1, enemyRects, hit);

        bool fallingClose = (pu.y > (playerYF - 60.0f)) && (std::abs(dx) < 160.0f);
        float score = totalTime + 0.001f * std::abs(dx);
        if (hitIdx != -1 && hit.t > 0.0f && hit.t < 1.0f) score += occlusionPenalty_env;
        if (fallingClose) score *= 0.6f;

        // Special priority for critical power-ups: defense restoration and extra life
        bool isCritical = (pu.type == 0 || pu.type == 2); // RestoreDefense=0, ExtraLife=2 (enum values as int)
        if (isCritical) {
            score *= 0.2f; // massive priority boost - make these 5x more attractive
            // Even if occluded, critical powerups are worth pursuing
            if (hitIdx != -1 && hit.t > 0.0f && hit.t < 1.0f) score -= occlusionPenalty_env * 0.8f; // reduce occlusion penalty
        }

        // penalize extremely long waits (but less for critical powerups)
        if (totalTime > 4.0f) {
            float timePenalty = (totalTime - 4.0f) * 0.5f;
            if (isCritical) timePenalty *= 0.3f; // reduce time penalty for critical items
            score += timePenalty;
        }
        
        // Additional penalty: discourage power-up pursuit when many enemies are alive (but not for critical ones)
        int aliveCount = 0;
        for (const auto &e : lastObs.enemies) {
            if (e.hp > 0) aliveCount++;
        }
        if (aliveCount > 10 && !isCritical) score += aliveCount * 0.1f; // penalty increases with enemy count, but skip for critical

        if (score < bestScore) { bestScore = score; bestPU = (int)i; }
    }

    // 3) PRIORITY 4: Decide: pursue PU only if it is quick enough and not leaving many urgent enemies
    auto countUrgentIfAbsent = [&](float horizon)->int {
        int cnt = 0;
        for (const auto &e : lastObs.enemies) {
            if (e.hp <= 0) continue;
            // Expanded urgency zone and reduced distance threshold for more aggressive enemy prioritization
            if (e.y > (interceptThresholdY - 120.0f)) {
                float distX = std::abs(e.x - playerXF);
                if (distX > 90.0f) cnt++; // reduced from 100.0f
            }
        }
        return cnt;
    };

    // Count total alive enemies to bias toward combat when many are present
    int aliveEnemies = 0;
    for (const auto &e : lastObs.enemies) {
        if (e.hp > 0) aliveEnemies++;
    }

    if (bestPU != -1) {
        float t = puTimes[bestPU];
        int urgent = countUrgentIfAbsent(t);
        // Check if the best power-up is critical (defense restoration or extra life)
        bool isCriticalPU = (lastObs.powerups[bestPU].type == 0 || lastObs.powerups[bestPU].type == 2);
        
        // More restrictive conditions: shorter time limit and bias against PU pursuit when many enemies alive
        bool manyEnemiesAlive = (aliveEnemies > 15); // if more than 15 enemies, prioritize combat
        
        // Allow pursuit if: normal conditions OR critical powerup (with relaxed conditions)
        bool normalPursuit = (urgent == 0 && t < 0.8f && !manyEnemiesAlive);
        bool criticalPursuit = (isCriticalPU && t < 2.0f && urgent <= 2); // allow critical PU pursuit even with some urgent enemies
        
        if (normalPursuit || criticalPursuit) {
            float dx = lastObs.powerups[bestPU].x - playerXF;
            if (dx < -6.0f) moveLeft = true;
            else if (dx > 6.0f) moveRight = true;
            // while moving, be ready to fire at nearby enemies
            for (const auto &e : lastObs.enemies) {
                if (e.hp <= 0) continue;
                if (std::abs(e.x - playerXF) < 80.0f || std::abs(e.x - lastObs.powerups[bestPU].x) < 60.0f) { fire = true; break; }
            }
        }
    }

    // 4) PRIORITY 5: Attack fallback: move toward nearest enemy column and shoot when unobstructed
    // Priority: always engage if there are enemies, even if we were considering a power-up
    if (!enemyRects.empty()) {
        int nearestOrig = -1; float nearestDist = 1e9f; int nearestRectIdx = -1;
        for (size_t j = 0; j < enemyIdx.size(); ++j) {
            int orig = enemyIdx[j]; const auto &e = lastObs.enemies[orig];
            float d = std::abs(e.x - playerXF);
            if (d < nearestDist) { nearestDist = d; nearestOrig = orig; nearestRectIdx = (int)j; }
        }
        if (nearestOrig != -1) {
            float targetX = lastObs.enemies[nearestOrig].x;
            float dx = targetX - playerXF;
            // More aggressive movement toward enemies
            if (dx < -3.0f) moveLeft = true; else if (dx > 3.0f) moveRight = true; // reduced threshold from 6.0f

            SDL_FPoint p0{ playerXF, playerYF };
            SDL_FPoint p1{ lastObs.enemies[nearestOrig].x, lastObs.enemies[nearestOrig].y };
            Core::Raycast::HitResult h2; int hitIdx2 = Core::Raycast::RaycastRects(p0, p1, enemyRects, h2);
            if (hitIdx2 != -1 && hitIdx2 == nearestRectIdx && h2.t > 0.0f && h2.t < 1.0f) fire = true;
            else if (nearestDist < 50.0f) fire = true; // increased from 30.0f for more aggressive firing
        }
    } else {
        // no enemies: if a PU is reasonable, move for it
        if (bestPU != -1 && puTimes[bestPU] < 2.0f) { // reduced from 3.0f
            float dx = lastObs.powerups[bestPU].x - playerXF;
            if (dx < -6.0f) moveLeft = true; else if (dx > 6.0f) moveRight = true;
        }
    }
}

#if 0
// Inline implementations are provided in the header; keep these commented out here.
bool AIController::WantsMoveLeft() const { return moveLeft; }
bool AIController::WantsMoveRight() const { return moveRight; }
bool AIController::WantsFire() const { return fire; }
bool AIController::WantsUseShield() const { return false; }
#endif

}} // namespace
