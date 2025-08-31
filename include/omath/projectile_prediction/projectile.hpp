//
// Created by Vlad on 6/9/2024.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"

namespace omath::projectile_prediction
{
    class Projectile final
    {
    public:
        Vector3<float> m_origin;
        float m_launch_speed{};
        float m_gravity_scale{};
    };
} // namespace omath::projectile_prediction