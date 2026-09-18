//
// Created by Vlad on 8/6/2025.
//
#pragma once
#include "omath/engines/frostbite_engine/formulas.hpp"
#include "omath/projectile_prediction/launcher.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"

namespace omath::frostbite_engine
{
    class PredEngineTrait final
    {
    public:
        [[nodiscard("projectile position result should not be discarded")]]
        constexpr static Vector3<float>
        predict_projectile_position(const Vector3<float>& launch_origin,
                                    const projectile_prediction::Projectile<float>& projectile, const float pitch,
                                    const float yaw, const float time, const float gravity) noexcept
        {
            auto current_pos = launch_origin
                               + forward_vector({PitchAngle::from_degrees(-pitch), YawAngle::from_degrees(yaw),
                                                 RollAngle::from_degrees(0)})
                                         * projectile.m_launch_speed * time;
            current_pos.y -= (gravity * projectile.m_gravity_scale) * (time * time) * 0.5f;

            return current_pos;
        }
        [[nodiscard("target position result should not be discarded")]]
        static constexpr Vector3<float> predict_target_position(const projectile_prediction::Target<float>& target,
                                                                const float time, const float gravity) noexcept
        {
            auto predicted = target.m_origin + target.m_velocity * time;

            if (target.m_is_airborne)
                predicted.y -= gravity * (time * time) * 0.5f;

            return predicted;
        }
        [[nodiscard("2d distance result should not be discarded")]]
        static float calc_vector_2d_distance(const Vector3<float>& delta) noexcept
        {
            return std::sqrt(delta.x * delta.x + delta.z * delta.z);
        }

        [[nodiscard("height coordinate result should not be discarded")]]
        constexpr static float get_vector_height_coordinate(const Vector3<float>& vec) noexcept
        {
            return vec.y;
        }

        [[nodiscard("view basis result should not be discarded")]]
        constexpr static projectile_prediction::ViewBasis<float> calc_view_basis(const float pitch,
                                                                                 const float yaw) noexcept
        {
            const auto rotation = rotation_matrix(
                    {PitchAngle::from_degrees(-pitch), YawAngle::from_degrees(yaw), RollAngle::from_degrees(0)});

            return {.forward = mat_rotate_vector(rotation, k_abs_forward),
                    .right = mat_rotate_vector(rotation, k_abs_right),
                    .up = mat_rotate_vector(rotation, k_abs_up)};
        }
        // Due to specification of maybe_calculate_projectile_launch_pitch_angle, pitch angle must be:
        // 89 look up, -89 look down
        [[nodiscard("pitch angle result should not be discarded")]]
        static float calc_direct_pitch_angle(const Vector3<float>& origin, const Vector3<float>& view_to) noexcept
        {
            const auto direction = (view_to - origin).normalized();
            return angles::radians_to_degrees(std::asin(direction.y));
        }
        [[nodiscard("yaw angle result should not be discarded")]]
        static float calc_direct_yaw_angle(const Vector3<float>& origin, const Vector3<float>& view_to) noexcept
        {
            const auto direction = (view_to - origin).normalized();

            return angles::radians_to_degrees(std::atan2(direction.x, direction.z));
        };
    };
} // namespace omath::frostbite_engine
