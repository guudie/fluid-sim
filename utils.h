#pragma once
#include <fstream>
#include <vector>
#include "glm/glm.hpp"

struct point {
    glm::vec2 pos;
    glm::vec2 vel;
    glm::vec2 acc;
    glm::ivec2 gridIdx;
    float density;
    float pressure;
    bool locked;
};

struct segment {
    point *p_ptr, *q_ptr;
    float len;
};

void cleanUtils();
void resolveOutOfBounds(point& p, int w, int h);
void resolveVelocity(const glm::vec2& p, glm::vec2& v, const int& height);