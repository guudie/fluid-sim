#include "implicitEuler.h"

static glm::vec2 defaultCap(glm::vec2 v) {
    return v;
}

implicitEuler::implicitEuler(utilFunc g, capFunc c) {
    assert(g != nullptr);
    f_func = nullptr;
    g_func = g;
    cap = c ? c : defaultCap;
}

void implicitEuler::integrate(glm::vec2& y, glm::vec2& z, glm::vec2 zdash, float dt, float t) {
    z += g_func(t, y, z, zdash) * dt;
    y += z * dt;
}

void implicitEuler::integrateStep1(glm::vec2& y, glm::vec2& z, glm::vec2 zdash, float dt, float t) {
    z += g_func(t, y, z, zdash) * dt;
}

void implicitEuler::integrateStep2(glm::vec2& y, glm::vec2 z, float dt, float t) {
    y += z * dt;
}