#include <iostream>
#include <sstream>
#include <libconfig.h++>
#include "utils.h"
#include "global.h"

// class utilsConfigEntity;

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
            parseConfig(instance->utilsconfig, utilsConfigPath);

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

void parseConfig(libconfig::Config& cfg, const char* configPath) {
    try {
        cfg.readFile(configPath);
    }
    catch(const libconfig::FileIOException &fioex)
    {
        throw "I/O error while reading file.";
    }
    catch(const libconfig::ParseException &pex)
    {
        std::stringstream ss;
        ss << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError();
        throw ss.str().c_str();
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