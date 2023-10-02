#include "ODESolver.h"

static glm::vec2 defaultFFunc(float t, glm::vec2 y, glm::vec2 z, glm::vec2 zdash) {
    return z;
}

static glm::vec2 defaultGFunc(float t, glm::vec2 y, glm::vec2 z, glm::vec2 zdash) {
    return zdash;
}

ODESolver::ODESolver(utilFunc f, utilFunc g) {
    f_func = f != nullptr ? f : defaultFFunc;
    g_func = g != nullptr ? g : defaultGFunc;
}

void ODESolver::integrateStep1(glm::vec2& y, glm::vec2& z, glm::vec2 zdash, float dt, float t) {
    // dummy function
}
void ODESolver::integrateStep2(glm::vec2& y, glm::vec2 z, float dt, float t) {
    // dummy function
}