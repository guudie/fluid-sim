#pragma once
#include "ODESolver.h"

class velocityVerlet : ODESolver {
public:
    velocityVerlet(utilFunc g);
    void Integrate(glm::vec2& y, glm::vec2& z, glm::vec2 zdash, float dt, float t = 0) override;
};