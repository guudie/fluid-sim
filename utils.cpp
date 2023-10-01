#include <iostream>
#include <libconfig.h++>
#include "utils.h"

static const std::string utilsConfigPath = "config/utils.cfg";

class utilsSingleton {
private:
    libconfig::Config utilsconfig;
    float bounceCoeff;
    float groundBounceCoeff;
    utilsSingleton() = default;

public:
    static utilsSingleton* instance;

    static utilsSingleton* getInstance() {
        if(!instance) {
            instance = new utilsSingleton();
            try {
                instance->utilsconfig.readFile(utilsConfigPath);
            }
            catch(const libconfig::FileIOException &fioex)
            {
                std::cerr << "I/O error while reading file." << std::endl;
            }
            catch(const libconfig::ParseException &pex)
            {
                std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError() << std::endl;
            }

            instance->bounceCoeff = instance->utilsconfig.lookup("bounce_coeff");
            instance->groundBounceCoeff = instance->utilsconfig.lookup("ground_bounce_coeff");
        }
        return instance;
    }

    static void clean() {
        if(instance)
            delete instance;
    }

    float getBounceCoeff() { return bounceCoeff; }
    float getGroundBounceCoeff() { return groundBounceCoeff; }
};
utilsSingleton* utilsSingleton::instance = nullptr;

void cleanUtils() {
    utilsSingleton::clean();
}

/*
    NONE,
    NAN_DENSITY,
    NAN_PRESSURE,
    NAN_ACC,
    NAN_POS,
    IDX_OUT_OF_RANGE
*/
void printErrorMessage(const error_code& err) {
    switch(err) {
    case error_code::NAN_DENSITY:
        std::cout << "Nan encountered in density" << std::endl;
        break;
    case error_code::NAN_PRESSURE:
        std::cout << "Nan encountered in pressure" << std::endl;
        break;
    case error_code::NAN_ACC:
        std::cout << "Nan encountered in acc" << std::endl;
        break;
    case error_code::NAN_POS:
        std::cout << "Nan encountered in position" << std::endl;
        break;
    case error_code::IDX_OUT_OF_RANGE:
        std::cout << "Index out of range" << std::endl;
        break;
    default:
        break;
    }
}

void resolveOutOfBounds(point& p, int w, int h) {
    utilsSingleton* instance = utilsSingleton::getInstance();
    if(p.pos.x > w) {
        p.pos.x = w;
        p.vel.x *= -instance->getBounceCoeff();
    }
    if(p.pos.x < 0){
        p.pos.x = 0;
        p.vel.x *= -instance->getBounceCoeff();
    }
    if(p.pos.y > h - 10) {
        p.pos.y = h - 10;
        p.vel.y *= -instance->getBounceCoeff() * instance->getGroundBounceCoeff();
    }
    if(p.pos.y < 0) {
        p.pos.y = 0;
        p.vel.y *= -instance->getBounceCoeff();
    }
}

void resolveVelocity(const glm::vec2& p, glm::vec2& v, const int& height) {
    if(abs(v.x) < 0.25f)
        v.x = 0;
    if(abs(v.y) < 0.25f && p.y > height - 10 - 3)
        v.y = 0;
    float len = glm::length(v);
    v *= len > 100.0f ? 100.0f / len : 1;
}