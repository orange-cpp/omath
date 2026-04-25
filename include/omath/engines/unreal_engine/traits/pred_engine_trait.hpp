//
// Created by Vlad on 8/6/2025.
//
#pragma once
#include "omath/engines/unreal_engine/formulas.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"
#include <optional>

namespace omath::unreal_engine
{
    class PredEngineTrait final
    {
    public:
        static Vector3<double> predict_projectile_position(const projectile_prediction::Projectile<double>& projectile,
                                                           const double pitch, const double yaw,
                                                           const double time, const double gravity) noexcept
        {
            const auto launch_pos = projectile.m_origin + projectile.m_launch_offset;
            const auto fwd_d = forward_vector({PitchAngle::from_degrees(-pitch), YawAngle::from_degrees(yaw),
                                               RollAngle::from_degrees(0)});
            auto current_pos = launch_pos
                               + Vector3<double>{fwd_d.x, fwd_d.y, fwd_d.z}
                                         * projectile.m_launch_speed * time;
            current_pos.y -= (gravity * projectile.m_gravity_scale) * (time * time) * 0.5;

            return current_pos;
        }

        [[nodiscard]]
        static Vector3<double> predict_target_position(const projectile_prediction::Target<double>& target,
                                                       const double time, const double gravity) noexcept
        {
            auto predicted = target.m_origin + target.m_velocity * time;

            if (target.m_is_airborne)
                predicted.y -= gravity * (time * time) * 0.5;

            return predicted;
        }

        [[nodiscard]]
        static double calc_vector_2d_distance(const Vector3<double>& delta) noexcept
        {
            return std::sqrt(delta.x * delta.x + delta.z * delta.z);
        }

        [[nodiscard]]
        static double get_vector_height_coordinate(const Vector3<double>& vec) noexcept
        {
            return vec.y;
        }

        [[nodiscard]]
        static Vector3<double> calc_viewpoint_from_angles(const projectile_prediction::Projectile<double>& projectile,
                                                          Vector3<double> predicted_target_position,
                                                          const std::optional<double> projectile_pitch) noexcept
        {
            const auto delta2d = calc_vector_2d_distance(predicted_target_position - projectile.m_origin);
            const auto height = delta2d * std::tan(angles::degrees_to_radians(projectile_pitch.value()));

            return {predicted_target_position.x, predicted_target_position.y, projectile.m_origin.z + height};
        }

        // Due to specification of maybe_calculate_projectile_launch_pitch_angle, pitch angle must be:
        // 89 look up, -89 look down
        [[nodiscard]]
        static double calc_direct_pitch_angle(const Vector3<double>& origin, const Vector3<double>& view_to) noexcept
        {
            const auto direction = (view_to - origin).normalized();

            return angles::radians_to_degrees(std::asin(direction.z));
        }

        [[nodiscard]]
        static double calc_direct_yaw_angle(const Vector3<double>& origin, const Vector3<double>& view_to) noexcept
        {
            const auto direction = (view_to - origin).normalized();

            return angles::radians_to_degrees(std::atan2(direction.y, direction.x));
        }
    };
} // namespace omath::unreal_engine
