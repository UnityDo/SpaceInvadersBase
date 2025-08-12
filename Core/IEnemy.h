#pragma once

class IEnemy {
public:
    virtual ~IEnemy() = default;
    virtual void Update(float deltaTime) = 0;
    virtual void TakeDamage(int amount) = 0;
    virtual bool IsAlive() const = 0;
};
