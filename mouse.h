#pragma once
#include "glm/glm.hpp"

class mouse {
private:
    glm::vec2 pos;
    glm::vec2 prevPos;

    bool leftButtonDown;
    bool rightButtonDown;
    bool sideButtonX2Down;

public:
    mouse() : pos({0, 0}), prevPos({0, 0}), leftButtonDown(false), rightButtonDown(false), sideButtonX2Down(false) { }
    ~mouse() = default;

    const glm::vec2& getPos() const { return pos; }
    const glm::vec2& getPrevPos() const { return prevPos; }
    glm::vec2 getDiff() const { return pos - prevPos; }
    void updatePos(const glm::vec2& _pos);
    void updatePos(int x, int y);

    bool getLB() const { return leftButtonDown; }
    void setLB(bool state) { leftButtonDown = state; }

    bool getRB() const { return rightButtonDown; }
    void setRB(bool state) { rightButtonDown = state; }

    bool getSBX2() const { return sideButtonX2Down; }
    void setSBX2(bool state) { sideButtonX2Down = state; }
};