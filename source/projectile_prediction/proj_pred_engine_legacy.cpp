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

            const auto projectile_pitch
                    = maybe_calculate_projectile_launch_pitch_angle(projectile, predicted_target_position);

            if (!projectile_pitch.has_value()) [[unlikely]]
                continue;

            if (!is_projectile_reached_target(predicted_target_position, projectile, projectile_pitch.value(), time))
                continue;

            const auto delta2d = (predicted_target_position - projectile.m_origin).length_2d();
            const auto height = delta2d * std::tan(angles::degrees_to_radians(projectile_pitch.value()));

            return Vector3(predicted_target_position.x, predicted_target_position.y, projectile.m_origin.z + height);
        }
        return std::nullopt;
    }

    std::optional<float>
    ProjPredEngineLegacy::maybe_calculate_projectile_launch_pitch_angle(const Projectile& projectile,
                                                                        const Vector3<float>& target_position) const
    {
        const auto bullet_gravity = m_gravity_constant * projectile.m_gravity_scale;
        const auto delta = target_position - projectile.m_origin;

        const auto distance2d = delta.length_2d();
        const auto distance2d_sqr = distance2d * distance2d;
        const auto launch_speed_sqr = projectile.m_launch_speed * projectile.m_launch_speed;

        float root = launch_speed_sqr * launch_speed_sqr
                     - bullet_gravity * (bullet_gravity * distance2d_sqr + 2.0f * delta.z * launch_speed_sqr);

        if (root < 0.0f) [[unlikely]]
            return std::nullopt;

        root = std::sqrt(root);
        const float angle = std::atan((launch_speed_sqr - root) / (bullet_gravity * distance2d));

        return angles::radians_to_degrees(angle);
    }

    bool ProjPredEngineLegacy::is_projectile_reached_target(const Vector3<float>& target_position,
                                                            const Projectile& projectile, const float pitch,
                                                            const float time) const
    {
        const auto yaw = projectile.m_origin.view_angle_to(target_position).y;
        const auto projectile_position = projectile.predict_position(pitch, yaw, time, m_gravity_constant);

        return projectile_position.distance_to(target_position) <= m_distance_tolerance;
    }
} // namespace omath::projectile_prediction
