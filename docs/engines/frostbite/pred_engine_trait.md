// Created by Vlad on 8/6/2025.
#pragma once

#include <cmath> // sqrt, hypot, tan, asin, atan2
#include <optional>

#include "omath/engines/frostbite_engine/formulas.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"

namespace omath::frostbite_engine
{
class PredEngineTrait final
{
public:
// Predict projectile position given launch angles (degrees), time (s), and world gravity (m/s^2).
// Note: kept runtime function; remove constexpr to avoid CTAD surprises across toolchains.
static Vector3<float> predict_projectile_position(
const projectile_prediction::Projectile& projectile,
float pitch_deg, float yaw_deg,
float time, float gravity) noexcept
{
// Engine convention: negative pitch looks up (your original used -pitch).
const auto fwd = forward_vector({
PitchAngle::from_degrees(-pitch_deg),
YawAngle::from_degrees(yaw_deg),
RollAngle::from_degrees(0.0f)
});

            Vector3<float> pos =
                projectile.m_origin +
                fwd * (projectile.m_launch_speed * time);

            // s = 1/2 a t^2 downward
            pos.y -= (gravity * projectile.m_gravity_scale) * (time * time) * 0.5f;
            return pos;
        }

        [[nodiscard]]
        static Vector3<float> predict_target_position(
            const projectile_prediction::Target& target,
            float time, float gravity) noexcept
        {
            Vector3<float> predicted = target.m_origin + target.m_velocity * time;

            if (target.m_is_airborne) {
                // If targets also have a gravity scale in your model, multiply here.
                predicted.y -= gravity * (time * time) * 0.5f;
            }
            return predicted;
        }

        [[nodiscard]]
        static float calc_vector_2d_distance(const Vector3<float>& delta) noexcept
        {
            // More stable than sqrt(x*x + z*z)
            return std::hypot(delta.x, delta.z);
        }

        [[nodiscard]]
        static float get_vector_height_coordinate(const Vector3<float>& vec) noexcept
        {
            return vec.y;
        }

        // Computes a viewpoint above the predicted target, using an optional projectile pitch.
        // If pitch is absent, we leave Y unchanged (or you can choose a sensible default).
        [[nodiscard]]
        static Vector3<float> calc_viewpoint_from_angles(
            const projectile_prediction::Projectile& projectile,
            const Vector3<float>& predicted_target_position,
            const std::optional<float> projectile_pitch_deg) noexcept
        {
            // Lateral separation from projectile to target (X/Z plane).
            const auto delta2d = calc_vector_2d_distance(predicted_target_position - projectile.m_origin);

            float y = predicted_target_position.y;
            if (projectile_pitch_deg.has_value()) {
                const float pitch_rad = angles::degrees_to_radians(*projectile_pitch_deg);
                const float height = delta2d * std::tan(pitch_rad);
                y += height;
            }

            // Use the target's Z, not the projectile's Z (likely bugfix).
            return { predicted_target_position.x, y, predicted_target_position.z };
        }

        // Due to maybe_calculate_projectile_launch_pitch_angle spec: +89° up, -89° down.
        [[nodiscard]]
        static float calc_direct_pitch_angle(const Vector3<float>& origin,
                                             const Vector3<float>& view_to) noexcept
        {
            const auto direction = (view_to - origin).normalized();
            return angles::radians_to_degrees(std::asin(direction.y));
        }

        [[nodiscard]]
        static float calc_direct_yaw_angle(const Vector3<float>& origin,
                                           const Vector3<float>& view_to) noexcept
        {
            const auto direction = (view_to - origin).normalized();
            return angles::radians_to_degrees(std::atan2(direction.x, direction.z));
        }
    };
} // namespace omath::frostbite_engine
