#pragma once
#include <functional>
#include <glm/glm.hpp>

class ODESolver {
public:
    typedef std::function<glm::vec2(float, glm::vec2, glm::vec2, glm::vec2)> utilFunc;

    ODESolver(utilFunc f = nullptr, utilFunc g = nullptr);
    virtual void Integrate(glm::vec2& y, glm::vec2& z, glm::vec2 zdash, float dt, float t = 0) = 0;

protected:
    utilFunc f_func;
    utilFunc g_func;
};