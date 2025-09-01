//
// Created by Vlad on 9/1/2025.
//

module;

#include <optional>

export module omath.projectile_prediction.proj_pred_engine;

export import omath.projectile_prediction.projectile;
export import omath.projectile_prediction.target;

namespace omath::projectile_prediction
{
    template<class T>
    concept PredEngineConcept =
            requires(const Projectile& projectile, const Target& target, const Vector3<float>& vec_a,
                     const Vector3<float>& vec_b,
                     Vector3<float> v3, // by-value for calc_viewpoint_from_angles
                     float pitch, float yaw, float time, float gravity, std::optional<float> maybe_pitch) {
                // Presence + return types
                {
                    T::predict_projectile_position(projectile, pitch, yaw, time, gravity)
                } -> std::same_as<Vector3<float>>;
                { T::predict_target_position(target, time, gravity) } -> std::same_as<Vector3<float>>;
                { T::calc_vector_2d_distance(vec_a) } -> std::same_as<float>;
                { T::get_vector_height_coordinate(vec_b) } -> std::same_as<float>;
                { T::calc_viewpoint_from_angles(projectile, v3, maybe_pitch) } -> std::same_as<Vector3<float>>;
                { T::calc_direct_pitch_angle(vec_a, vec_b) } -> std::same_as<float>;
                { T::calc_direct_yaw_angle(vec_a, vec_b) } -> std::same_as<float>;

                // Enforce noexcept as in PredEngineTrait
                requires noexcept(T::predict_projectile_position(projectile, pitch, yaw, time, gravity));
                requires noexcept(T::predict_target_position(target, time, gravity));
                requires noexcept(T::calc_vector_2d_distance(vec_a));
                requires noexcept(T::get_vector_height_coordinate(vec_b));
                requires noexcept(T::calc_viewpoint_from_angles(projectile, v3, maybe_pitch));
                requires noexcept(T::calc_direct_pitch_angle(vec_a, vec_b));
                requires noexcept(T::calc_direct_yaw_angle(vec_a, vec_b));
            };
    template<class EngineTrait = source_engine::PredEngineTrait>
    requires PredEngineConcept<EngineTrait>
    class ProjPredEngineLegacy final : public ProjPredEngineInterface
    {
    public:
        explicit ProjPredEngineLegacy(const float gravity_constant, const float simulation_time_step,
                                      const float maximum_simulation_time, const float distance_tolerance)
            : m_gravity_constant(gravity_constant), m_simulation_time_step(simulation_time_step),
              m_maximum_simulation_time(maximum_simulation_time), m_distance_tolerance(distance_tolerance)
        {
        }

        [[nodiscard]]
        std::optional<Vector3<float>> maybe_calculate_aim_point(const Projectile& projectile,
                                                                const Target& target) const override
        {
            for (float time = 0.f; time < m_maximum_simulation_time; time += m_simulation_time_step)
            {
                const auto predicted_target_position =
                        EngineTrait::predict_target_position(target, time, m_gravity_constant);

                const auto projectile_pitch =
                        maybe_calculate_projectile_launch_pitch_angle(projectile, predicted_target_position);

                if (!projectile_pitch.has_value()) [[unlikely]]
                    continue;

                if (!is_projectile_reached_target(predicted_target_position, projectile, projectile_pitch.value(),
                                                  time))
                    continue;

                return EngineTrait::calc_viewpoint_from_angles(projectile, predicted_target_position, projectile_pitch);
            }
            return std::nullopt;
        }

    private:
        const float m_gravity_constant;
        const float m_simulation_time_step;
        const float m_maximum_simulation_time;
        const float m_distance_tolerance;

        // Realization of this formula:
        // https://stackoverflow.com/questions/54917375/how-to-calculate-the-angle-to-shoot-a-bullet-in-order-to-hit-a-moving-target
        /*
        \[
        \theta \;=\; \arctan\!\Biggl(
        \frac{%
        v^{2}\;\pm\;\sqrt{\,v^{4}-g\!\left(gx^{2}+2yv^{2}\right)\,}
        }{%
        gx
       }\Biggr)
       \]
        */
        [[nodiscard]]
        std::optional<float>
        maybe_calculate_projectile_launch_pitch_angle(const Projectile& projectile,
                                                      const Vector3<float>& target_position) const noexcept
        {
            const auto bullet_gravity = m_gravity_constant * projectile.m_gravity_scale;

            if (bullet_gravity == 0.f)
                return EngineTrait::calc_direct_pitch_angle(projectile.m_origin, target_position);

            const auto delta = target_position - projectile.m_origin;

            const auto distance2d = EngineTrait::calc_vector_2d_distance(delta);
            const auto distance2d_sqr = distance2d * distance2d;
            const auto launch_speed_sqr = projectile.m_launch_speed * projectile.m_launch_speed;

            float root = launch_speed_sqr * launch_speed_sqr
                         - bullet_gravity
                                   * (bullet_gravity * distance2d_sqr
                                      + 2.0f * EngineTrait::get_vector_height_coordinate(delta) * launch_speed_sqr);

            if (root < 0.0f) [[unlikely]]
                return std::nullopt;

            root = std::sqrt(root);
            const float angle = std::atan((launch_speed_sqr - root) / (bullet_gravity * distance2d));

            return angles::radians_to_degrees(angle);
        }
        [[nodiscard]]
        bool is_projectile_reached_target(const Vector3<float>& target_position, const Projectile& projectile,
                                          const float pitch, const float time) const noexcept
        {
            const auto yaw = EngineTrait::calc_direct_yaw_angle(projectile.m_origin, target_position);
            const auto projectile_position =
                    EngineTrait::predict_projectile_position(projectile, pitch, yaw, time, m_gravity_constant);

            return projectile_position.distance_to(target_position) <= m_distance_tolerance;
        }
    };
} // namespace omath::projectile_prediction

#include "omath/engines/source_engine/traits/pred_engine_trait.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include "omath/projectile_prediction/proj_pred_engine.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"
#include <optional>

namespace omath::projectile_prediction
{
    template<class T>
    concept PredEngineConcept =
            requires(const Projectile& projectile, const Target& target, const Vector3<float>& vec_a,
                     const Vector3<float>& vec_b,
                     Vector3<float> v3, // by-value for calc_viewpoint_from_angles
                     float pitch, float yaw, float time, float gravity, std::optional<float> maybe_pitch) {
                // Presence + return types
                {
                    T::predict_projectile_position(projectile, pitch, yaw, time, gravity)
                } -> std::same_as<Vector3<float>>;
                { T::predict_target_position(target, time, gravity) } -> std::same_as<Vector3<float>>;
                { T::calc_vector_2d_distance(vec_a) } -> std::same_as<float>;
                { T::get_vector_height_coordinate(vec_b) } -> std::same_as<float>;
                { T::calc_viewpoint_from_angles(projectile, v3, maybe_pitch) } -> std::same_as<Vector3<float>>;
                { T::calc_direct_pitch_angle(vec_a, vec_b) } -> std::same_as<float>;
                { T::calc_direct_yaw_angle(vec_a, vec_b) } -> std::same_as<float>;

                // Enforce noexcept as in PredEngineTrait
                requires noexcept(T::predict_projectile_position(projectile, pitch, yaw, time, gravity));
                requires noexcept(T::predict_target_position(target, time, gravity));
                requires noexcept(T::calc_vector_2d_distance(vec_a));
                requires noexcept(T::get_vector_height_coordinate(vec_b));
                requires noexcept(T::calc_viewpoint_from_angles(projectile, v3, maybe_pitch));
                requires noexcept(T::calc_direct_pitch_angle(vec_a, vec_b));
                requires noexcept(T::calc_direct_yaw_angle(vec_a, vec_b));
            };
    template<class EngineTrait = source_engine::PredEngineTrait>
    requires PredEngineConcept<EngineTrait>
    class ProjPredEngineLegacy final : public ProjPredEngineInterface
    {
    public:
        explicit ProjPredEngineLegacy(const float gravity_constant, const float simulation_time_step,
                                      const float maximum_simulation_time, const float distance_tolerance)
            : m_gravity_constant(gravity_constant), m_simulation_time_step(simulation_time_step),
              m_maximum_simulation_time(maximum_simulation_time), m_distance_tolerance(distance_tolerance)
        {
        }

        [[nodiscard]]
        std::optional<Vector3<float>> maybe_calculate_aim_point(const Projectile& projectile,
                                                                const Target& target) const override
        {
            for (float time = 0.f; time < m_maximum_simulation_time; time += m_simulation_time_step)
            {
                const auto predicted_target_position =
                        EngineTrait::predict_target_position(target, time, m_gravity_constant);

                const auto projectile_pitch =
                        maybe_calculate_projectile_launch_pitch_angle(projectile, predicted_target_position);

                if (!projectile_pitch.has_value()) [[unlikely]]
                    continue;

                if (!is_projectile_reached_target(predicted_target_position, projectile, projectile_pitch.value(),
                                                  time))
                    continue;

                return EngineTrait::calc_viewpoint_from_angles(projectile, predicted_target_position, projectile_pitch);
            }
            return std::nullopt;
        }

    private:
        const float m_gravity_constant;
        const float m_simulation_time_step;
        const float m_maximum_simulation_time;
        const float m_distance_tolerance;

        // Realization of this formula:
        // https://stackoverflow.com/questions/54917375/how-to-calculate-the-angle-to-shoot-a-bullet-in-order-to-hit-a-moving-target
        /*
        \[
        \theta \;=\; \arctan\!\Biggl(
        \frac{%
        v^{2}\;\pm\;\sqrt{\,v^{4}-g\!\left(gx^{2}+2yv^{2}\right)\,}
        }{%
        gx
       }\Biggr)
       \]
        */
        [[nodiscard]]
        std::optional<float>
        maybe_calculate_projectile_launch_pitch_angle(const Projectile& projectile,
                                                      const Vector3<float>& target_position) const noexcept
        {
            const auto bullet_gravity = m_gravity_constant * projectile.m_gravity_scale;

            if (bullet_gravity == 0.f)
                return EngineTrait::calc_direct_pitch_angle(projectile.m_origin, target_position);

            const auto delta = target_position - projectile.m_origin;

            const auto distance2d = EngineTrait::calc_vector_2d_distance(delta);
            const auto distance2d_sqr = distance2d * distance2d;
            const auto launch_speed_sqr = projectile.m_launch_speed * projectile.m_launch_speed;

            float root = launch_speed_sqr * launch_speed_sqr
                         - bullet_gravity
                                   * (bullet_gravity * distance2d_sqr
                                      + 2.0f * EngineTrait::get_vector_height_coordinate(delta) * launch_speed_sqr);

            if (root < 0.0f) [[unlikely]]
                return std::nullopt;

            root = std::sqrt(root);
            const float angle = std::atan((launch_speed_sqr - root) / (bullet_gravity * distance2d));

            return angles::radians_to_degrees(angle);
        }
        [[nodiscard]]
        bool is_projectile_reached_target(const Vector3<float>& target_position, const Projectile& projectile,
                                          const float pitch, const float time) const noexcept
        {
            const auto yaw = EngineTrait::calc_direct_yaw_angle(projectile.m_origin, target_position);
            const auto projectile_position =
                    EngineTrait::predict_projectile_position(projectile, pitch, yaw, time, m_gravity_constant);

            return projectile_position.distance_to(target_position) <= m_distance_tolerance;
        }
    };
} // namespace omath::projectile_prediction
