#pragma once
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <libconfig.h++>
#include "glm/glm.hpp"

struct point;

class renderer;
class mouse;
class ODESolver;

class fluid_sim {
private:
    std::unordered_set<point*>** grid;
    std::vector<point*> points;
    renderer* _renderer = nullptr;
    mouse* _mouse = nullptr;
    ODESolver* _integrator;

    bool running = false;

    Uint32 lastUpdateTime;
    Uint32 currentTime;
    Uint32 tickDuration;

    int generateCount = 0;
    int maxGenerateCount = 6;
    float dt = 1.0f;
    float radius = 4.0f;
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
    fluid_sim() = default;
    ~fluid_sim() = default;

    bool isRunning() const;
    Uint32 getTickDuration() const;

    mouse* const& getMouseObject() const;
    float getRadius() const;
    float getH() const;

    void setup(const libconfig::Config& cfg, int windowWidth, int windowHeight, ODESolver* integrator);
    bool checkShouldUpdate();
    void input();
    void postInput();
    void generateParticles(const glm::ivec2& from, const glm::ivec2& to, float dist);
    void generateInitialParticles();
    void calcDensityAndPressure();
    void calcAcceleration();
    void integrateMovements();
    void update();
    void render();
    void destroy();
};