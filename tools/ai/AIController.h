#pragma once
#include "../../Core/IPlayerController.h"
#include <random>
#include <chrono>
#include <iostream>
#include "../../Core/Raycast.h"

namespace tools { namespace ai {

class AIController : public IPlayerController {
public:
    AIController(unsigned int seed = 0) {
        if (seed == 0) seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
        rng.seed(seed);
    }
    void Update(float dt) override {
        // Política mejorada contra pasividad en perseguir power-ups:
        // - Detectar enemigos "urgentes" (cercanos a las defensas) y darles prioridad.
        // - Calcular coste de oportunidad antes de comprometerse a un power-up.
        // - Mientras se desplaza hacia un power-up, disparar a enemigos cercanos en X (no esperar alineación perfecta).
        moveLeft = moveRight = fire = false;
        float interceptThresholdY = 480.0f; // zona baja de pantalla (urgencia)
        float playerXf = playerX;

        // 1) Buscar enemigos urgentes y reaccionar inmediatamente
        bool didIntercept = false;
        for (const auto& e : lastObs.enemies) {
            if (e.y > interceptThresholdY) {
                float dx = e.x - playerXf;
                if (std::abs(dx) < 140.0f) {
                    if (dx < -6.0f) moveLeft = true;
                    else if (dx > 6.0f) moveRight = true;
                    fire = true; // disparar agresivamente
                    didIntercept = true;
                    break;
                }
            }
        }
        if (didIntercept) return;

    // 2) Evaluar power-ups: hallar el más alcanzable y su tiempo estimado
    // Nota: no tenemos rects en WorldObservation, así que asumimos un hitbox para enemigos
    // que actúa como bloqueador en el raycast (ancho x alto). Si el raycast encuentra
    // un enemigo entre el jugador y el powerup, consideramos el powerup como "ocluido".
    // Suposición razonable: enemigo hitbox ~ 44x30 (puede ajustarse más tarde).
        float bestPUscore = 1e9f; int bestPU = -1;
        std::vector<float> puTotalTime(lastObs.powerups.size());

        // Construir una sola vez los rects de enemigos vivos (ignoramos defensas)
        const float enemyW = 44.0f, enemyH = 30.0f;
        std::vector<SDL_FRect> enemyRects; enemyRects.reserve(lastObs.enemies.size());
        std::vector<int> enemyMap; enemyMap.reserve(lastObs.enemies.size());
        for (size_t ei = 0; ei < lastObs.enemies.size(); ++ei) {
            const auto &e = lastObs.enemies[ei];
            if (e.hp <= 0) continue;
            enemyMap.push_back((int)ei);
            enemyRects.push_back(SDL_FRect{ e.x - enemyW*0.5f, e.y - enemyH*0.5f, enemyW, enemyH });
        }
        for (size_t i=0;i<lastObs.powerups.size();++i) {
            float dx = lastObs.powerups[i].x - playerXf;
            float dy = lastObs.powerups[i].y - lastObs.playerY; // vertical to player
            float timeToReachPU = std::abs(dy) / 120.0f; // vy ~120
            float horizontalDist = std::abs(dx);
            float timeToMove = horizontalDist / 300.0f; // player speed ~300
            float totalTime = timeToReachPU + timeToMove;
            puTotalTime[i] = totalTime;

            // Determinar si el powerup está "cerca y cayendo" (preferir)
            bool fallingClose = (lastObs.powerups[i].y > (lastObs.playerY - 60.0f)) && (std::abs(dx) < 160.0f);

#if 1
            Core::Raycast::HitResult hit;
            SDL_FPoint p0{ playerXf, lastObs.playerY };
            SDL_FPoint p1{ lastObs.powerups[i].x, lastObs.powerups[i].y };
            int hitIndex = Core::Raycast::RaycastRects(p0, p1, enemyRects, hit);
#else
            // previous: use templated getter over EnemyInfo
#endif
#if defined(AI_DEBUG)
            if (hitIndex != -1) std::cout << "[AI] Raycast hit enemy index=" << hitIndex << " t=" << hit.t << "\n";
            else std::cout << "[AI] Raycast no hit for PU " << i << "\n";
#endif

            // score = tiempo + pequeña penalización por distancia horizontal
            float score = totalTime + (horizontalDist * 0.001f);
            // penalizar fuertemente si está ocluido por un enemigo
            if (hitIndex != -1 && hit.t > 0.0f && hit.t < 1.0f) {
                score += 10.0f; // muy poco atractivo
            }
            // favorecer powerups cercanos y cayendo
            if (fallingClose) score *= 0.6f;

            if (score < bestPUscore) { bestPUscore = score; bestPU = (int)i; }
        }

        // 2.a) Calcular enemigos "urgentes" que quedarían sin atención si vamos por el PU
        auto countUrgentEnemies = [&](float horizonSeconds)->int {
            int cnt = 0;
            for (const auto& e : lastObs.enemies) {
                if (e.y > (interceptThresholdY - 80.0f)) {
                    // si está lejos del jugador y no en la columna del PU, cuenta como urgente
                    float distX = std::abs(e.x - playerXf);
                    if (distX > 100.0f) cnt++;
                }
            }
            return cnt;
        };

        if (bestPU != -1) {
            float totalTime = puTotalTime[bestPU];
            // Si recoger el powerup tarda mucho y hay enemigos urgentes, no comprometerse
            int urgent = countUrgentEnemies(totalTime);
            if (urgent == 0 || totalTime < 1.2f) {
                // perseguir powerup, pero disparar mientras nos movemos
                float dx = lastObs.powerups[bestPU].x - playerXf;
                if (dx < -6.0f) moveLeft = true;
                else if (dx > 6.0f) moveRight = true;

                // mientras nos movemos hacia el PU, disparar si hay enemigos en rango horizontal razonable
                for (const auto& e : lastObs.enemies) {
                    if (std::abs(e.x - playerXf) < 80.0f || std::abs(e.x - lastObs.powerups[bestPU].x) < 60.0f) {
                        fire = true; break;
                    }
                }
                // no bloqueamos aquí: la lógica de ataque puede evaluarse también
            }
            // si hay urgencias y totalTime es grande, dejaremos que la sección de ataque las maneje
        }

        // 3) Ataque general: priorizar columnas con enemigos reales (no columnas vacías)
        if (!enemyRects.empty()) {
            // Elegir el enemigo más cercano en X al jugador
            int nearestOrigIdx = -1; float nearestDist = 1e9f; int nearestRectIdx = -1;
            for (size_t j = 0; j < enemyMap.size(); ++j) {
                int orig = enemyMap[j];
                const auto &e = lastObs.enemies[orig];
                float d = std::abs(e.x - playerXf);
                if (d < nearestDist) { nearestDist = d; nearestOrigIdx = orig; nearestRectIdx = (int)j; }
            }
            // mover hacia la columna del enemigo más cercano
            if (nearestOrigIdx != -1) {
                float targetX = lastObs.enemies[nearestOrigIdx].x;
                float dxCenter = targetX - playerXf;
                if (dxCenter < -6.0f) moveLeft = true;
                else if (dxCenter > 6.0f) moveRight = true;

                // decidir disparo: usar raycast hacia ese enemigo y disparar solo si el raycast alcanza ese rect
                SDL_FPoint p0{ playerXf, lastObs.playerY };
                SDL_FPoint p1{ lastObs.enemies[nearestOrigIdx].x, lastObs.enemies[nearestOrigIdx].y };
                Core::Raycast::HitResult hit2;
                int hitIdx = Core::Raycast::RaycastRects(p0, p1, enemyRects, hit2);
                if (hitIdx != -1 && hitIdx == nearestRectIdx && hit2.t > 0.0f && hit2.t < 1.0f) {
                    // podemos disparar con confianza
                    fire = true;
                } else {
                    // si no hay línea directa, intentar disparar a enemigos muy cercanos horizontalmente
                    if (nearestDist < 30.0f) fire = true;
                    else fire = false;
                }
            }
        } else {
            // no hay enemigos: solo mover hacia powerups si los hay
            if (bestPU != -1 && puTotalTime[bestPU] < 3.0f) {
                float dx = lastObs.powerups[bestPU].x - playerXf;
                if (dx < -6.0f) moveLeft = true;
                else if (dx > 6.0f) moveRight = true;
            }
        }

        // 4) Seguridad: si estamos persiguiendo un power-up pero hay enemigos urgentes apareciendo, priorizarlos
        if (bestPU != -1) {
            float totalTime = puTotalTime[bestPU];
            for (const auto &e : lastObs.enemies) {
                if (e.y > (interceptThresholdY - 40.0f) && std::abs(e.x - playerXf) < 160.0f && totalTime > 1.0f) {
                    // abandonar persecución de PU y centrar ataque
                    // mover hacia el enemigo urgente
                    if (e.x - playerXf < -6.0f) { moveLeft = true; moveRight = false; }
                    else if (e.x - playerXf > 6.0f) { moveRight = true; moveLeft = false; }
                    fire = true;
                    break;
                }
            }
        }
    }
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
    // Simple policy state
    bool moveLeft = false;
    bool moveRight = false;
    bool fire = false;
};

}} // namespace
