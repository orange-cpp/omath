//
// Created by Vlad on 8/6/2025.
//
#pragma once
#include "omath/engines/cry_engine/formulas.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"
#include <optional>

namespace omath::cry_engine
{
    class PredEngineTrait final
    {
    public:
        constexpr static Vector3<float> predict_projectile_position(const projectile_prediction::Projectile& projectile,
                                                                    const float pitch, const float yaw,
                                                                    const float time, const float gravity) noexcept
        {
            auto current_pos = projectile.m_origin
                               + forward_vector({PitchAngle::from_degrees(-pitch), YawAngle::from_degrees(yaw),
                                                 RollAngle::from_degrees(0)})
                                         * projectile.m_launch_speed * time;
            current_pos.z -= (gravity * projectile.m_gravity_scale) * (time * time) * 0.5f;

            return current_pos;
        }
        [[nodiscard]]
        static constexpr Vector3<float> predict_target_position(const projectile_prediction::Target& target,
                                                                const float time, const float gravity) noexcept
        {
            auto predicted = target.m_origin + target.m_velocity * time;

            if (target.m_is_airborne)
                predicted.z -= gravity * (time * time) * 0.5f;

            return predicted;
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
        static Vector3<float> calc_viewpoint_from_angles(const projectile_prediction::Projectile& projectile,
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
            const auto direction = (view_to - origin).normalized();
            return angles::radians_to_degrees(std::asin(direction.z));
        }
        [[nodiscard]]
        static float calc_direct_yaw_angle(const Vector3<float>& origin, const Vector3<float>& view_to) noexcept
        {
            const auto direction = (view_to - origin).normalized();

            return angles::radians_to_degrees(-std::atan2(direction.x, direction.y));
        };
    };
} // namespace omath::cry_engine
