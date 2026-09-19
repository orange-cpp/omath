//
// Created by Vlad on 9/19/2026.
//

#pragma once
#include "omath/engines/source_engine/traits/pred_engine_trait.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include "omath/projectile_prediction/launcher.hpp"
#include "omath/projectile_prediction/pred_engine_concept.hpp"
#include "omath/projectile_prediction/proj_pred_engine.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/trigonometry/angles.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace omath::projectile_prediction
{
    // One round in flight, advanced a physics step at a time. Drag has no closed form, so a round that has it can
    // only be followed the way the game's physics moves it: every step adds gravity, then scales the whole velocity
    // down by the drag for that step, then moves, in that order and without sub-steps. That is what Source's
    // VPhysics does to a grenade, and stepping at its tick interval follows the real thing to within a couple of
    // units over a two second flight, where a parabola is hundreds out by then.
    //
    // A round without drag is a parabola, which PredEngineTrait::predict_projectile_position() reads off directly.
    template<class EngineTrait = source_engine::PredEngineTrait, class ArithmeticType = float>
    requires PredEngineConcept<EngineTrait, ArithmeticType>
    class ProjectileFlight final
    {
    public:
        // view_angles are the shooter's, pitch positive upwards: the muzzle is placed from them and the round leaves
        // launcher.launch_pitch_offset above them. gravity_constant is the world's, scaled here by the round's own
        // m_gravity_scale. time_step is the physics step of the game being predicted, not an accuracy setting: the
        // arc only matches when it is stepped the way the game steps it.
        ProjectileFlight(const Projectile<ArithmeticType>& projectile, const Launcher<ArithmeticType>& launcher,
                         const AimAngles<ArithmeticType>& view_angles, const ArithmeticType gravity_constant,
                         const ArithmeticType time_step) noexcept
            : m_projectile(projectile), m_gravity(gravity_constant * projectile.m_gravity_scale), m_time_step(time_step)
        {
            const auto basis = EngineTrait::calc_view_basis(view_angles.pitch, view_angles.yaw);

            m_origin = launcher.launch_origin(basis);
            m_position = m_origin;
            m_forward = basis.forward;
            m_up = basis.up;

            // A level view's up is the world's, whichever axis the engine keeps it on
            m_world_up = EngineTrait::calc_view_basis(ArithmeticType{0}, ArithmeticType{0}).up;

            // forward * a + up * b in the view frame, written back out of the speed and offset they are kept as
            const auto offset = angles::degrees_to_radians(launcher.launch_pitch_offset);
            m_velocity = basis.forward * (projectile.m_launch_speed * std::cos(offset))
                         + basis.up * (projectile.m_launch_speed * std::sin(offset));

            clamp_speed();
        }

        void step() noexcept
        {
            m_velocity = m_velocity - m_world_up * (m_gravity * m_time_step);

            const auto rate = m_projectile.m_drag * m_velocity.length()
                              + m_projectile.m_drag_forward * std::abs(m_velocity.dot(m_forward))
                              + m_projectile.m_drag_up * std::abs(m_velocity.dot(m_up));

            // A step cannot take more than the whole velocity
            m_velocity = m_velocity * (ArithmeticType{1} - std::min(ArithmeticType{1}, rate * m_time_step));

            clamp_speed();

            m_position = m_position + m_velocity * m_time_step;
            ++m_step_count;
        }

        // Where the round left from: the muzzle, not the eye
        [[nodiscard]]
        const Vector3<ArithmeticType>& origin() const noexcept
        {
            return m_origin;
        }

        [[nodiscard]]
        const Vector3<ArithmeticType>& position() const noexcept
        {
            return m_position;
        }

        [[nodiscard]]
        const Vector3<ArithmeticType>& velocity() const noexcept
        {
            return m_velocity;
        }

        // step * count rather than a running sum, which drifts
        [[nodiscard]]
        ArithmeticType time() const noexcept
        {
            return m_time_step * static_cast<ArithmeticType>(m_step_count);
        }

    private:
        void clamp_speed() noexcept
        {
            if (!(m_projectile.m_max_speed > ArithmeticType{0}))
                return;

            const auto speed = m_velocity.length();

            if (speed > m_projectile.m_max_speed)
                m_velocity = m_velocity * (m_projectile.m_max_speed / speed);
        }

        Projectile<ArithmeticType> m_projectile;
        ArithmeticType m_gravity;
        ArithmeticType m_time_step;

        Vector3<ArithmeticType> m_origin;
        Vector3<ArithmeticType> m_position;
        Vector3<ArithmeticType> m_velocity;

        // The round's own axes. It is spawned facing along the view and nothing in flight turns it, so for a round
        // that does not tumble these stay where they were at launch.
        Vector3<ArithmeticType> m_forward;
        Vector3<ArithmeticType> m_up;

        Vector3<ArithmeticType> m_world_up;

        std::size_t m_step_count = 0;
    };
} // namespace omath::projectile_prediction
