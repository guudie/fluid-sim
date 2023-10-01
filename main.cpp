#define SDL_MAIN_HANDLED
#include <iostream>
#include <vector>
#include <unordered_set>
#include <SDL2/SDL.h>
#include <omp.h>
#include <libconfig.h++>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "ODE_solvers/implicitEuler.h"
#include "renderer.h"
#include "mouse.h"
#include "utils.h"
#include "fluid_sim.h"

int main() {
    const int width = 512, height = 512;
    std::string configPath = "config/general.cfg";
    std::string utilsConfigPath = "config/utils.cfg";

    libconfig::Config cfg;
    try {
        cfg.readFile(configPath);
    }
    catch(const libconfig::FileIOException &fioex)
    {
        std::cerr << "I/O error while reading file." << std::endl;
        return EXIT_FAILURE;
    }
    catch(const libconfig::ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError() << std::endl;
        return EXIT_FAILURE;
    }

    float dt = cfg.lookup("dt");
    glm::vec2 G;
    G.x = cfg.lookup("gravity.x");
    G.y = cfg.lookup("gravity.y");

    implicitEuler _integrator([=](float t, glm::vec2 y, glm::vec2 z, glm::vec2 zdash) -> glm::vec2 {
        return zdash + G;
    });

    fluid_sim* sim = new fluid_sim();
    sim->setup(cfg, width, height, &_integrator);

    glm::ivec2 tl(0, 450);
    glm::ivec2 br(width-1, height-11);
    float dist = sim->getH() - 0.0001f;

    sim->generateParticles(tl, br, dist);

    tl = { 200, 100 };
    br = { 350, 175 };
    int cnt = 0;

    Uint32 lastUpd = SDL_GetTicks();
    while(sim->isRunning()) {
        Uint32 curTime = SDL_GetTicks();
        if(curTime - lastUpd >= 16) {
            sim->input();
            if(sim->getMouseObject()->getRB() && cnt < 6) {
                sim->generateParticles(tl, br, sim->getH());
                sim->getMouseObject()->setRB(false);
                cnt++;
            }

            try {
                sim->update();
            } catch(const char* err) {
                std::cout << err << std::endl;
                break;
            }

            sim->render();

            lastUpd = curTime;
        }
    }

    sim->destroy();
    cleanUtils();
    delete sim;

    std::cout << "Quit program" << std::endl;

    return 0;
}