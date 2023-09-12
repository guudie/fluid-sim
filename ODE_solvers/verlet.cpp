#include "verlet.h"

static glm::vec2 defaultCap(glm::vec2 v) {
    return v;
}

verlet::verlet(capFunc c, utilFunc f, utilFunc g) : ODESolver(f, g) {
    cap = c ? c : defaultCap;
}

void verlet::integrate(glm::vec2& y, glm::vec2& z, glm::vec2 zdash, float dt, float t) {
    glm::vec2 prev = y;
    y += cap(y - z) + g_func(t, y, z, zdash) * dt * dt;  // does not need to apply forces here and zdash can be used instead
    z = prev;
}