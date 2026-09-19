//
// Created by Vlad on 6/9/2024.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"

namespace omath::projectile_prediction
{
    // Physical properties of the round. Where it is fired from is described by Launcher.
    template<class ArithmeticType = float>
    class Projectile final
    {
    public:
        // Read only by the compatibility wrappers of ProjPredEngineInterface: m_origin is the eye and m_launch_offset
        // a fixed world-space spawn offset. New code leaves them default and passes a Launcher instead.
        Vector3<ArithmeticType> m_origin{};
        Vector3<ArithmeticType> m_launch_offset{};

        ArithmeticType m_launch_speed{};
        ArithmeticType m_gravity_scale{};
    };
} // namespace omath::projectile_prediction