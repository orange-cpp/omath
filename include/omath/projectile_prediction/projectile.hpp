//
// Created by Vlad on 6/9/2024.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"

namespace omath::projectile_prediction
{
    template <class ArithmeticType = float>
    class Projectile final
    {
    public:
        Vector3<ArithmeticType> m_origin;
        Vector3<ArithmeticType> m_launch_offset{};
        ArithmeticType m_launch_speed{};
        ArithmeticType m_gravity_scale{};
    };
} // namespace omath::projectile_prediction