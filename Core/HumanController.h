#pragma once
#include "IPlayerController.h"
#include "InputManager.h"

class HumanController : public IPlayerController {
public:
    HumanController(InputManager* im) : input(im) {}
    void Update(float dt) override { /* nothing to update; input manager state is polled externally */ }
    bool WantsMoveLeft() const override { return input && input->IsLeftPressed(); }
    bool WantsMoveRight() const override { return input && input->IsRightPressed(); }
    bool WantsFire() const override { return input && input->IsFirePressed(); }
    bool WantsUseShield() const override { return false; }
private:
    InputManager* input;
};
