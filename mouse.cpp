#include "mouse.h"

const glm::vec2& mouse::getPos() const {
    return pos;
}

const glm::vec2& mouse::getPrevPos() const {
    return prevPos;
}

glm::vec2 mouse::getDiff() const {
    return pos - prevPos;
}

void mouse::updatePos(const glm::vec2& _pos) {
    prevPos = pos;
    pos = _pos;
}

void mouse::updatePos(int x, int y) {
    prevPos = pos;
    pos.x = x;
    pos.y = y;
}

bool mouse::getLB() const {
    return leftButtonDown;
}

void mouse::setLB(bool state) {
    leftButtonDown = state;
}

bool mouse::getRB() const {
    return rightButtonDown;
}

void mouse::setRB(bool state) {
    rightButtonDown = state;
}

bool mouse::getSBX2() const {
    return sideButtonX2Down;
}

void mouse::setSBX2(bool state) {
    sideButtonX2Down = state;
}