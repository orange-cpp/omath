#include "omath/projectile_prediction/proj_pred_engine_legacy.hpp"
#include <cmath>
#include <omath/angles.hpp>

namespace omath::projectile_prediction
{
    ProjPredEngineLegacy::ProjPredEngineLegacy(const float gravity_constant, const float simulation_time_step,
                                               const float maximum_simulation_time, const float distance_tolerance)
        : m_gravity_constant(gravity_constant), m_simulation_time_step(simulation_time_step),
          m_maximum_simulation_time(maximum_simulation_time), m_distance_tolerance(distance_tolerance)
    {
    }

    std::optional<Vector3<float>> ProjPredEngineLegacy::maybe_calculate_aim_point(const Projectile& projectile,
                                                                                  const Target& target) const
    {
        for (float time = 0.f; time < m_maximum_simulation_time; time += m_simulation_time_step)
        {
            const auto predicted_target_position = target.predict_position(time, m_gravity_constant);

            const auto projectile_pitch =
                    maybe_calculate_projectile_launch_pitch_angle(projectile, predicted_target_position);

            if (!projectile_pitch.has_value()) [[unlikely]]
                continue;

            if (!is_projectile_reached_target(predicted_target_position, projectile, projectile_pitch.value(), time))
                continue;

            return calc_viewpoint_from_angles(projectile, predicted_target_position, projectile_pitch);
        }
        return std::nullopt;
    }

    std::optional<float> ProjPredEngineLegacy::maybe_calculate_projectile_launch_pitch_angle(
            const Projectile& projectile, const Vector3<float>& target_position) const noexcept
    {
        const auto bullet_gravity = m_gravity_constant * projectile.m_gravity_scale;
        const auto delta = target_position - projectile.m_origin;

        const auto distance2d = calc_vector_2d_distance(delta);
        const auto distance2d_sqr = distance2d * distance2d;
        const auto launch_speed_sqr = projectile.m_launch_speed * projectile.m_launch_speed;

        float root = launch_speed_sqr * launch_speed_sqr
                     - bullet_gravity
                               * (bullet_gravity * distance2d_sqr
                                  + 2.0f * get_vector_height_coordinate(delta) * launch_speed_sqr);

        if (root < 0.0f) [[unlikely]]
            return std::nullopt;

        root = std::sqrt(root);
        const float angle = std::atan((launch_speed_sqr - root) / (bullet_gravity * distance2d));

        return angles::radians_to_degrees(angle);
    }

    bool ProjPredEngineLegacy::is_projectile_reached_target(const Vector3<float>& target_position,
                                                            const Projectile& projectile, const float pitch,
                                                            const float time) const noexcept
    {
        const auto yaw = projectile.m_origin.view_angle_to(target_position).y;
        const auto projectile_position = predict_projectile_position(projectile, pitch, yaw, time, m_gravity_constant);

        return projectile_position.distance_to(target_position) <= m_distance_tolerance;
    }

    float ProjPredEngineLegacy::calc_vector_2d_distance(const Vector3<float>& delta) const
    {
        return std::sqrt(delta.x * delta.x + delta.y * delta.y);
    }

    float ProjPredEngineLegacy::get_vector_height_coordinate(const Vector3<float>& vec) const
    {
        return vec.z;
    }
    Vector3<float> ProjPredEngineLegacy::calc_viewpoint_from_angles(const Projectile& projectile,
                                                                    const Vector3<float> predicted_target_position,
                                                                    const std::optional<float> projectile_pitch) const
    {
        const auto delta2d = calc_vector_2d_distance(predicted_target_position - projectile.m_origin);
        const auto height = delta2d * std::tan(angles::degrees_to_radians(projectile_pitch.value()));

        return {predicted_target_position.x, predicted_target_position.y, projectile.m_origin.z + height};
    }
    Vector3<float> ProjPredEngineLegacy::predict_projectile_position(const Projectile& projectile, const float pitch,
                                                                     const float yaw, const float time,
                                                                     const float gravity) const
    {
        auto current_pos = projectile.m_origin
                           + source_engine::forward_vector({source_engine::PitchAngle::from_degrees(-pitch),
                                                            source_engine::YawAngle::from_degrees(yaw),
                                                            source_engine::RollAngle::from_degrees(0)})
                                     * projectile.m_launch_speed * time;
        current_pos.z -= (gravity * projectile.m_gravity_scale) * (time * time) * 0.5f;

        return current_pos;
    }
} // namespace omath::projectile_prediction
