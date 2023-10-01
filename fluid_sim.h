#pragma once
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <libconfig.h++>

struct point;

class renderer;
class mouse;
class ODESolver;

class fluid_sim {
private:
    std::unordered_set<point*>** grid;
    renderer* _renderer = nullptr;
    mouse* _mouse = nullptr;
    ODESolver* _integrator;

    bool running = false;
    bool updateEveryTick;
    bool updatedThisTick = false;

    Uint32 lastUpdateTime;
    Uint32 currentTime;
    Uint32 tickDuration;

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

    void setup(const libconfig::Config& cfg, int windowWidth, int windowHeight, ODESolver* integrator);
    void input();
    void updateNoTick();
    void updateWithTick();
    void update();
    void render();
    void destroy();
};