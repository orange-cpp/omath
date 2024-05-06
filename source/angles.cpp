//
// Created by vlad on 11/6/23.
//

#include "uml/angles.h"
#include <numbers>


namespace uml::angles
{
    float RadToDeg(float rads)
    {
        return rads * 180.f / std::numbers::pi_v<float>;
    }

    float DegToRad(float degrees)
    {
        return degrees * std::numbers::pi_v<float> / 180.f;
    }
}
