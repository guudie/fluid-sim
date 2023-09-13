#pragma once
#include "ODESolver.h"

class implicitEuler : ODESolver {
public:
    implicitEuler(utilFunc g);
    void integrate(glm::vec2& y, glm::vec2& z, glm::vec2 zdash, float dt, float t = 0) override;
};