#pragma once
#include "ODESolver.h"

class implicitEuler : ODESolver {
public:
    implicitEuler(utilFunc g);
    void integrate(glm::vec2& y, glm::vec2& z, glm::vec2 zdash, float dt, float t = 0) override;

    // for separating the calculation of z and y when iterating through all particles
    void integrateStep1(glm::vec2& y, glm::vec2& z, glm::vec2 zdash, float dt, float t = 0) override;
    void integrateStep2(glm::vec2& y, glm::vec2 z, float dt, float t = 0) override;
};