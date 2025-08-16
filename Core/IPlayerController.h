#pragma once

#include "Observations.h"

class IPlayerController {
public:
    virtual ~IPlayerController() = default;
    virtual void Update(float dt) = 0;
    virtual bool WantsMoveLeft() const = 0;
    virtual bool WantsMoveRight() const = 0;
    virtual bool WantsFire() const = 0;
    virtual bool WantsUseShield() const = 0;
    virtual void Observe(const WorldObservation& obs) { (void)obs; }
};
