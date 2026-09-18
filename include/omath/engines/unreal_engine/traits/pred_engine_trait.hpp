//
// Created by Vlad on 8/6/2025.
//
#pragma once
#include "omath/engines/unreal_engine/formulas.hpp"
#include "omath/projectile_prediction/launcher.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"

namespace omath::unreal_engine
{
    class PredEngineTrait final
    {
    public:
        [[nodiscard("projectile position result should not be discarded")]]
        static Vector3<double> predict_projectile_position(const Vector3<double>& launch_origin,
                                                           const projectile_prediction::Projectile<double>& projectile,
                                                           const double pitch, const double yaw, const double time,
                                                           const double gravity) noexcept
        {
            const auto fwd_d = forward_vector(
                    {PitchAngle::from_degrees(pitch), YawAngle::from_degrees(yaw), RollAngle::from_degrees(0)});
            auto current_pos =
                    launch_origin + Vector3<double>{fwd_d.x, fwd_d.y, fwd_d.z} * projectile.m_launch_speed * time;
            current_pos.z -= (gravity * projectile.m_gravity_scale) * (time * time) * 0.5;

            return current_pos;
        }

        [[nodiscard("target position result should not be discarded")]]
        static Vector3<double> predict_target_position(const projectile_prediction::Target<double>& target,
                                                       const double time, const double gravity) noexcept
        {
            auto predicted = target.m_origin + target.m_velocity * time;

            if (target.m_is_airborne)
                predicted.z -= gravity * (time * time) * 0.5;

            return predicted;
        }

        [[nodiscard("2d distance result should not be discarded")]]
        static double calc_vector_2d_distance(const Vector3<double>& delta) noexcept
        {
            return std::sqrt(delta.x * delta.x + delta.y * delta.y);
        }

        [[nodiscard("height coordinate result should not be discarded")]]
        static double get_vector_height_coordinate(const Vector3<double>& vec) noexcept
        {
            return vec.z;
        }

        [[nodiscard("view basis result should not be discarded")]]
        constexpr static projectile_prediction::ViewBasis<double> calc_view_basis(const double pitch,
                                                                                  const double yaw) noexcept
        {
            const auto rotation = rotation_matrix(
                    {PitchAngle::from_degrees(pitch), YawAngle::from_degrees(yaw), RollAngle::from_degrees(0)});

            return {.forward = mat_rotate_vector(rotation, k_abs_forward),
                    .right = mat_rotate_vector(rotation, k_abs_right),
                    .up = mat_rotate_vector(rotation, k_abs_up)};
        }

        // Due to specification of maybe_calculate_projectile_launch_pitch_angle, pitch angle must be:
        // 89 look up, -89 look down
        [[nodiscard("pitch angle result should not be discarded")]]
        static double calc_direct_pitch_angle(const Vector3<double>& origin, const Vector3<double>& view_to) noexcept
        {
            const auto direction = (view_to - origin).normalized();

            return angles::radians_to_degrees(std::asin(direction.z));
        }

        [[nodiscard("yaw angle result should not be discarded")]]
        static double calc_direct_yaw_angle(const Vector3<double>& origin, const Vector3<double>& view_to) noexcept
        {
            const auto direction = (view_to - origin).normalized();

            return angles::radians_to_degrees(std::atan2(direction.y, direction.x));
        }
    };
} // namespace omath::unreal_engine
