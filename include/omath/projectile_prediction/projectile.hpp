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

        // Air drag, in 1 / distance. Only ProjPredEngineDrag and ProjectileFlight fly a round with it: the closed-form
        // engines assume a parabola and ignore these. Each simulation step takes
        // dt * (m_drag * |v| + m_drag_forward * |v . F| + m_drag_up * |v . U|) off the velocity, with F and U the
        // round's own forward and up axes, which are those of the view it was fired from.
        //
        // m_drag is for a round that tumbles, showing the air every side in turn, which averages out to one number.
        // The other two are for a round that holds its attitude, where end-on and side-on drag differ and the arc
        // bending away from the nose moves the round from one to the other. There is no third axis because a round
        // fired without roll never moves along its own right.
        ArithmeticType m_drag{};
        ArithmeticType m_drag_forward{};
        ArithmeticType m_drag_up{};

        // Speed the physics engine caps the round at, on launch and again after every step (Source's VPhysics stops
        // at 2000 units/s). Zero means there is no cap. Read by the same two classes as the drag.
        ArithmeticType m_max_speed{};

        [[nodiscard("You must use drag check result")]]
        constexpr bool has_drag() const noexcept
        {
            return m_drag > ArithmeticType{0} || m_drag_forward > ArithmeticType{0} || m_drag_up > ArithmeticType{0};
        }
    };
} // namespace omath::projectile_prediction