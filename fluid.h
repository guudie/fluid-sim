#pragma once
#include <iostream>
#include <vector>
#include <unordered_set>

struct point;

class renderer;
class ODESolver;
class mouse;

class cloth {
private:
    std::unordered_set<point*>** grid;

    ODESolver* _integrator;
    
    int num_iterations;
    float K;
    float h;
    float h2;
    float h6;
    float p0;
    float e;
    float poly6_coeff;
    float spiky_coeff;
    float viscosity_lap_coeff;
    float mass;
    float max_vel;
    float max_acc;
    float mouse_coeff;
    int cellSize;
    int gridDimX;
    int gridDimY;

public:
    cloth(ODESolver* integrator);
    ~cloth();

    void init();
    void initFromFile(const std::string& path);
    void update(mouse* _mouse, int width, int height, float dt = 1);
    void drawAllParticles(renderer* _renderer);
};