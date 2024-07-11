//
// Created by vlad on 11/6/23.
//

#include "omath/angles.h"
#include <numbers>


namespace omath::angles
{
    float RadiansToDegrees(const float radiands)
    {
        return radiands * (180.f / std::numbers::pi_v<float>);
    }

    float DegreesToRadians(const float degrees)
    {
        return degrees * (std::numbers::pi_v<float> / 180.f);
    }
}
