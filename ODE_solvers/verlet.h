#pragma once
#include <functional>
#include <glm/glm.hpp>
#include "ODESolver.h"

class verlet : ODESolver {
public:
    typedef std::function<glm::vec2(glm::vec2)> capFunc;

    verlet(capFunc c = nullptr, utilFunc f = nullptr, utilFunc g = nullptr);
    void integrate(glm::vec2& y, glm::vec2& z, glm::vec2 zdash, float dt, float t = 0) override;

private:
    capFunc cap;
};