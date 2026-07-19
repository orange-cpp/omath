//
// Created by Vlad on 8/3/2025.
//

#pragma once
#include "omath/engines/source_engine/formulas.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"
#include <cmath>
#include <limits>
#include <optional>

namespace omath::source_engine
{
    class PredEngineTrait final
    {
    public:
        [[nodiscard("projectile position result should not be discarded")]]
        constexpr static Vector3<float>
        predict_projectile_position(const projectile_prediction::Projectile<float>& projectile, const float pitch,
                                    const float yaw, const float time, const float gravity) noexcept
        {
            const auto launch_pos = projectile.m_origin + projectile.m_launch_offset;
            const auto pitch_angle = PitchAngle::from_degrees(-pitch);
            const auto yaw_angle = YawAngle::from_degrees(yaw);
            const auto pitch_cos = pitch_angle.cos();
            // Roll is always zero here, so this is the exact first column of the rotation matrix.
            const Vector3 forward{pitch_cos * yaw_angle.cos(), pitch_cos * yaw_angle.sin(), -pitch_angle.sin()};
            auto current_pos = launch_pos + forward * projectile.m_launch_speed * time;
            current_pos.z -= (gravity * projectile.m_gravity_scale) * (time * time) * 0.5f;

            return current_pos;
        }

        [[nodiscard("reachability result should not be discarded")]]
        static constexpr bool
        can_projectile_reach_target_at_time(const projectile_prediction::Projectile<float>& projectile,
                                            const Vector3<float>& target_position, const float time,
                                            const float gravity, const float distance_tolerance) noexcept
        {
            if (!(distance_tolerance >= 0.f))
                return false;

            // After undoing gravity, every possible projectile position is on a sphere with radius speed * time.
            const auto launch_position = projectile.m_origin + projectile.m_launch_offset;
            auto adjusted_delta = target_position - launch_position;
            const auto gravity_displacement = (gravity * projectile.m_gravity_scale) * (time * time) * 0.5f;
            adjusted_delta.z += gravity_displacement;

            const auto target_distance_sqr = adjusted_delta.length_sqr();
            const auto projectile_distance = std::abs(projectile.m_launch_speed * time);
            const auto floating_point_margin =
                    std::numeric_limits<float>::epsilon() * 8.f
                    * (std::abs(target_position.x) + std::abs(target_position.y) + std::abs(target_position.z)
                       + std::abs(launch_position.x) + std::abs(launch_position.y) + std::abs(launch_position.z)
                       + std::abs(gravity_displacement) + projectile_distance + distance_tolerance + 1.f);

            if (!std::isfinite(target_distance_sqr) || !std::isfinite(floating_point_margin)) [[unlikely]]
                return true;

            const auto conservative_tolerance = distance_tolerance + floating_point_margin;
            const auto maximum_distance = projectile_distance + conservative_tolerance;
            if (target_distance_sqr > maximum_distance * maximum_distance)
                return false;

            const auto minimum_distance =
                    projectile_distance > conservative_tolerance ? projectile_distance - conservative_tolerance : 0.f;
            return target_distance_sqr >= minimum_distance * minimum_distance;
        }

        [[nodiscard("target position result should not be discarded")]]
        static constexpr Vector3<float> predict_target_position(const projectile_prediction::Target<float>& target,
                                                                const float time, const float gravity) noexcept
        {
            auto predicted = target.m_origin + target.m_velocity * time;

            if (target.m_is_airborne)
                predicted.z -= gravity * (time * time) * 0.5f;

            return predicted;
        }
        [[nodiscard("2d distance result should not be discarded")]]
        static float calc_vector_2d_distance(const Vector3<float>& delta) noexcept
        {
            return std::sqrt(delta.x * delta.x + delta.y * delta.y);
        }

        [[nodiscard("height coordinate result should not be discarded")]]
        constexpr static float get_vector_height_coordinate(const Vector3<float>& vec) noexcept
        {
            return vec.z;
        }

        [[nodiscard("viewpoint result should not be discarded")]]
        static Vector3<float> calc_viewpoint_from_angles(const projectile_prediction::Projectile<float>& projectile,
                                                         Vector3<float> predicted_target_position,
                                                         const std::optional<float> projectile_pitch) noexcept
        {
            const auto delta2d = calc_vector_2d_distance(predicted_target_position - projectile.m_origin);
            const auto height = delta2d * std::tan(angles::degrees_to_radians(projectile_pitch.value()));

            return {predicted_target_position.x, predicted_target_position.y, projectile.m_origin.z + height};
        }
        // Due to specification of maybe_calculate_projectile_launch_pitch_angle, pitch angle must be:
        // 89 look up, -89 look down
        [[nodiscard("pitch angle result should not be discarded")]]
        static float calc_direct_pitch_angle(const Vector3<float>& origin, const Vector3<float>& view_to) noexcept
        {
            const auto distance = origin.distance_to(view_to);
            const auto delta = view_to - origin;

            return angles::radians_to_degrees(std::asin(delta.z / distance));
        }
        [[nodiscard("yaw angle result should not be discarded")]]
        static float calc_direct_yaw_angle(const Vector3<float>& origin, const Vector3<float>& view_to) noexcept
        {
            const auto delta = view_to - origin;

            return angles::radians_to_degrees(std::atan2(delta.y, delta.x));
        };
    };
} // namespace omath::source_engine
