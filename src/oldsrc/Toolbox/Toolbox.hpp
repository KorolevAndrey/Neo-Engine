/* Toolbox class
 * Provides various utility functions */
#pragma once
#ifndef _TOOLBOX_HPP_
#define _TOOLBOX_HPP_

#include "glm/glm.hpp"
#include <stdlib.h>
#include <string>
#include <algorithm>

// TODO : Rewrite this so it's up to C++ standards
// TODO : templates

class Toolbox {
public:
    /* PI */
    static constexpr double PI = 3.14159265359;

    /* Generate a random float [0, 1] */
    static inline float genRandom() {
        return rand() / (float) RAND_MAX;
    }
    
    /* Generate a scaled random value */
    static inline float genRandom(const float val) {
        return genRandom() * val;
    }
   
    /* Generate a random value in a range [min, max] */
    static inline float genRandom(const float min, const float max) {
        return genRandom() * (max - min) + min;
    }

    /* Generate a random vec3 with values [0, 1] */
    static inline glm::vec3 genRandomVec3() {
        return glm::vec3(genRandom(), genRandom(), genRandom());
    }
    
    /* Generate random vec3 with values [0, 1] */
    static inline glm::vec3 genRandomVec3(const float min, const float max) {
        return glm::vec3(genRandom(min, max), genRandom(min, max), genRandom(min, max));
    }

    /* Generate string representation of a vector */
    static inline std::string vectorToString(const glm::vec3 in) {
        return "<" + std::to_string(in.x) + ", " + 
                     std::to_string(in.y) + ", " + 
                     std::to_string(in.z) + ">";

    }
    static inline std::string vectorToString(const glm::vec2 in) {
        return "<" + std::to_string(in.x) + ", " + 
                     std::to_string(in.y) + ">";

    }

    /* Clamp */
    static inline float clamp(float &input, const float min, const float max) {
        input = std::max(min, input);
        input = std::min(max, input);
        return input;
    }

    /* Interpolate */
    template<class T>
    static T interpolate(const T a, const T b, const float t) {
        return a * (1.f - t) + b * t;
    }
};


#endif