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
        std::string typeStr = enemyData.value("type", "basic");
        float x = enemyData.value("x", 0.0f);
        float y = enemyData.value("y", 0.0f);
        int hp = enemyData.value("hp", 1);
        float speed = enemyData.value("speed", 1.0f);
        int damage = enemyData.value("damage", 1);
        std::string patternStr = enemyData.value("pattern", "straight");

        EnemyType type = EnemyType::Basic;
        EnemyColor color(255,0,0,255);
        MovePattern pattern = MovePattern::Straight;

        if (typeStr == "basic") { type = EnemyType::Basic; color = EnemyColor(255,0,0,255); }
        else if (typeStr == "fast") { type = EnemyType::Fast; color = EnemyColor(0,0,255,255); }
        else if (typeStr == "tank") { type = EnemyType::Tank; color = EnemyColor(0,255,0,255); }
        else if (typeStr == "boss") { type = EnemyType::Boss; color = EnemyColor(255,255,0,255); }
        else if (typeStr == "sniper") { type = EnemyType::Sniper; color = EnemyColor(255,0,255,255); }
        else if (typeStr == "splitter") { type = EnemyType::Splitter; color = EnemyColor(255,128,0,255); }

        if (patternStr == "straight") pattern = MovePattern::Straight;
        else if (patternStr == "zigzag") pattern = MovePattern::ZigZag;
        else if (patternStr == "diagonal") pattern = MovePattern::Diagonal;
        else if (patternStr == "dive") pattern = MovePattern::Dive;
        else if (patternStr == "descend-stop-shoot") pattern = MovePattern::DescendStopShoot;
        else if (patternStr == "circle") pattern = MovePattern::Circle;
        else if (patternStr == "stationary") pattern = MovePattern::Stationary;
        else if (patternStr == "scatter") pattern = MovePattern::Scatter;

        Enemy enemy(x, y, hp, color, type, speed, damage, pattern);
        enemies.push_back(enemy);
        count++;
    }
    std::cout << "[EnemyFactory] Enemigos cargados para el nivel " << levelIndex << ": " << count << std::endl;
    return enemies;
}
