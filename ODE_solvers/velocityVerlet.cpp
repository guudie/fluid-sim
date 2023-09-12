#include "velocityVerlet.h"

velocityVerlet::velocityVerlet(utilFunc g) {
    assert(g != nullptr);
    f_func = nullptr;
    g_func = g;
}

void velocityVerlet::integrate(glm::vec2& y, glm::vec2& z, glm::vec2 zdash, float dt, float t) {
    y += z * dt + zdash * dt * dt * 0.5f;
    z += 0.5f * (zdash + g_func(t + dt, y, z, zdash)) * dt; // MUST compute and apply forces here
}