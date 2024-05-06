//
// Created by vlad on 11/6/23.
//
#define _USE_MATH_DEFINES
#include "uml/angles.h"
#include <cmath>

namespace uml::angles
{
    float RadToDeg(float rads)
    {
        return rads * 180.f / static_cast<float>(M_PI);
    }

    float DegToRad(float degrees)
    {
        return degrees * static_cast<float>(M_PI) / 180.f;
    }
}
