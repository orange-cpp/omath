//
// Created by Vlad on 6/9/2024.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"

namespace omath::projectile_prediction
{
    class Target final
    {
    public:
        Vector3<float> m_origin;
        Vector3<float> m_velocity;
        bool m_is_airborne{};
    };
} // namespace omath::projectile_prediction