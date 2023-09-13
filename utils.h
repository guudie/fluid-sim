#pragma once
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

struct point {
    glm::vec2 pos;
    glm::vec2 vel;
    glm::vec2 acc;
    bool locked;
};

struct segment {
    point *p_ptr, *q_ptr;
    float len;
};

inline void resolveOutOfBounds(point& p, int w, int h) {
    if(p.pos.x > w) {
        p.pos.x = w;
        p.vel.x *= -0.8f;
    }
    if(p.pos.x < 0){
        p.pos.x = 0;
        p.vel.x *= -0.8f;
    }
    if(p.pos.y > h - 10) {
        p.pos.y = h - 10;
        p.vel.y *= -0.8f;
    }
    if(p.pos.y < 0) {
        p.pos.y = 0;
        p.vel.y *= -0.8f;
    }
}

inline void resolveVelocity(const glm::vec2& p, glm::vec2& v, const int& height) {
    if(abs(v.x) < 0.25f)
        v.x = 0;
    if(abs(v.y) < 0.25f && p.y > height - 10 - 3)
        v.y = 0;
    float len = glm::length(v);
    v *= len > 100.0f ? 100.0f / len : 1;
}