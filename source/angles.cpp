//
// Created by vlad on 11/6/23.
//

#include "omath/angles.h"
#include <numbers>


namespace omath::angles
{
    constexpr float RadiansToDegrees(const float radiands)
    {
        return radiands * (180.f / std::numbers::pi_v<float>);
    }

    constexpr float DegreesToRadians(const float degrees)
    {
        return degrees * (std::numbers::pi_v<float> / 180.f);
    }
}
