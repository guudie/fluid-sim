#pragma once
#include <SDL2/SDL.h>
#include <iostream>

class renderer;
class mouse;
class fluid;
class ODESolver;

class application {
private:
    renderer* _renderer = nullptr;
    mouse* _mouse = nullptr;
    cloth* _cloth = nullptr;

    bool running = false;
    bool updateEveryTick;
    bool updatedThisTick = false;

    Uint32 lastUpdateTime;
    Uint32 currentTime;
    Uint32 tickDuration;

public:
    application(bool _updateEveryTick = false, Uint32 _tickDuration = 16) : updateEveryTick(_updateEveryTick), tickDuration(_tickDuration) { }
    ~application() = default;

    bool isRunning() const;
    bool isUpdateEveryTick() const;

    void setTickUpdate(bool _updateEveryTick = false, Uint32 _tickDuration = 16);

    mouse* const& getMouseObject() const;

    void setupFromPath(int windowWidth, int windowHeight, std::string path, ODESolver* _integrator);
    void input();
    void updateNoTick();
    void updateWithTick();
    void update();
    void render();
    void destroy();
};