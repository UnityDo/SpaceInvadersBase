#pragma once
#include "IEnemy.h"
#include "Enemy.h"
#include <vector>
#include <string>
#include <memory>

class EnemyFactory {
public:
    // Crea una lista de enemigos a partir de un archivo JSON de niveles y un índice de nivel
    static std::vector<Enemy> CreateEnemiesFromLevels(const std::string& jsonFilePath, int levelIndex = 0);
};
