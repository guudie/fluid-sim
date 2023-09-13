#include "mouse.h"

void mouse::updatePos(const glm::vec2& _pos) {
    prevPos = pos;
    pos = _pos;
}

void mouse::updatePos(int x, int y) {
    prevPos = pos;
    pos.x = x;
    pos.y = y;
}