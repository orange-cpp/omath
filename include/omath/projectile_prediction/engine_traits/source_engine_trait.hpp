//
// Created by Vlad on 8/3/2025.
//

#pragma once
#include "omath/engines/source_engine/formulas.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include <optional>

namespace omath::projectile_prediction::traits
{
    class SourceEngineTrait final
    {
    public:
        constexpr static Vector3<float> predict_projectile_position(const Projectile& projectile, const float pitch,
                                                                    const float yaw, const float time,
                                                                    const float gravity) noexcept
        {
            auto current_pos = projectile.m_origin
                               + source_engine::forward_vector({source_engine::PitchAngle::from_degrees(-pitch),
                                                                source_engine::YawAngle::from_degrees(yaw),
                                                                source_engine::RollAngle::from_degrees(0)})
                                         * projectile.m_launch_speed * time;
            current_pos.z -= (gravity * projectile.m_gravity_scale) * (time * time) * 0.5f;

            return current_pos;
        }

        static bool is_projectile_reached_target(const Vector3<float>& target_position,
                                                           const Projectile& projectile, const float pitch,
                                                           const float time, const float gravity,
                                                           const float tolerance) noexcept
        {
            const auto yaw = projectile.m_origin.view_angle_to(target_position).y;
            const auto projectile_position = predict_projectile_position(projectile, pitch, yaw, time, gravity);

            return projectile_position.distance_to(target_position) <= tolerance;
        }
        [[nodiscard]]
        static float calc_vector_2d_distance(const Vector3<float>& delta) noexcept
        {
            return std::sqrt(delta.x * delta.x + delta.y * delta.y);
        }

        [[nodiscard]]
        constexpr static float get_vector_height_coordinate(const Vector3<float>& vec) noexcept
        {
            return vec.z;
        }

        [[nodiscard]]
        static Vector3<float> calc_viewpoint_from_angles(const Projectile& projectile,
                                                         Vector3<float> predicted_target_position,
                                                         const std::optional<float> projectile_pitch) noexcept
        {
            const auto delta2d = calc_vector_2d_distance(predicted_target_position - projectile.m_origin);
            const auto height = delta2d * std::tan(angles::degrees_to_radians(projectile_pitch.value()));

            return {predicted_target_position.x, predicted_target_position.y, projectile.m_origin.z + height};
        }
        // Due to specification of maybe_calculate_projectile_launch_pitch_angle, pitch angle must be:
        // 89 look up, -89 look down
        [[nodiscard]]
        static float calc_direct_pitch_angle(const Vector3<float>& origin, const Vector3<float>& view_to) noexcept
        {
            const auto distance = origin.distance_to(view_to);
            const auto delta = view_to - origin;

            return angles::radians_to_degrees(std::asin(delta.z / distance));
        }
    };
} // namespace omath::projectile_prediction::traits