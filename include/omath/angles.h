//
// Created by vlad on 11/6/23.
//

#pragma once
#include <numbers>

namespace omath::angles
{
    [[nodiscard]] constexpr float RadiansToDegrees(const float radiands)
    {
        return radiands * (180.f / std::numbers::pi_v<float>);
    }
    [[nodiscard]] constexpr float DegreesToRadians(const float degrees)
    {
        return degrees * (std::numbers::pi_v<float> / 180.f);
    }
}
