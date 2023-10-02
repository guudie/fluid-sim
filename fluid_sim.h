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
class implicitEuler;

class fluid_sim {
private:
    std::unordered_set<point*>** grid;
    std::vector<point*> points;
    renderer* _renderer = nullptr;
    mouse* _mouse = nullptr;
    implicitEuler* _integrator;

    bool running = false;
    bool updateEveryTick;
    bool updatedThisTick = false;

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
    fluid_sim(bool _updateEveryTick = false, Uint32 _tickDuration = 16) : updateEveryTick(_updateEveryTick), tickDuration(_tickDuration) { }
    ~fluid_sim() = default;

    bool isRunning() const;
    bool isUpdateEveryTick() const;

    void setTickUpdate(bool _updateEveryTick = false, Uint32 _tickDuration = 16);

    mouse* const& getMouseObject() const;
    float getRadius() const;
    float getH() const;

    void setup(const libconfig::Config& cfg, int windowWidth, int windowHeight, implicitEuler* integrator);
    void input();
    void generateParticles(const glm::ivec2& from, const glm::ivec2& to, float dist);
    void generateInitialParticles();
    void calcDensityAndPressure();
    void calcAcceleration();
    void integrateMovements();
    void update();
    void render();
    void destroy();
};