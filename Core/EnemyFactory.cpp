#include "EnemyFactory.h"
#include "Enemy.h"
#include <iostream>
#include <fstream>
#include "../libs/nlohmann/json.hpp"

using json = nlohmann::json;

std::vector<Enemy> EnemyFactory::CreateEnemiesFromLevels(const std::string& jsonFilePath, int levelIndex) {
    std::vector<Enemy> enemies;
    std::ifstream file(jsonFilePath);
    if (!file.is_open()) {
        std::cout << "[EnemyFactory] No se pudo abrir el archivo: " << jsonFilePath << std::endl;
        return enemies;
    }
    json data;
    file >> data;
    if (!data.contains("levels")) {
        std::cout << "[EnemyFactory] El archivo no contiene 'levels'." << std::endl;
        return enemies;
    }
    if (levelIndex < 0 || levelIndex >= data["levels"].size()) {
        std::cout << "[EnemyFactory] Índice de nivel fuera de rango: " << levelIndex << std::endl;
        return enemies;
    }
    const auto& level = data["levels"][levelIndex];
    if (!level.contains("enemies")) {
        std::cout << "[EnemyFactory] El nivel no contiene 'enemies'." << std::endl;
        return enemies;
    }
    int count = 0;
    for (const auto& enemyData : level["enemies"]) {
        std::string type = enemyData.value("type", "basic");
        float x = enemyData.value("x", 0.0f);
        float y = enemyData.value("y", 0.0f);
        int hp = enemyData.value("hp", 1);
        EnemyColor color(255,0,0,255); // default rojo
        if (type == "fast") color = EnemyColor(0,0,255,255); // azul
        else if (type == "tank") color = EnemyColor(0,255,0,255); // verde
        Enemy enemy(x, y, hp, color);
        enemy.color = color; // refuerzo explícito
        enemies.push_back(enemy);
        count++;
    }
    std::cout << "[EnemyFactory] Enemigos cargados para el nivel " << levelIndex << ": " << count << std::endl;
    return enemies;
}
