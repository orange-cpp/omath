//
// Created by Vlad on 6/9/2024.
//

#pragma once

#include "omath/engines/source_engine/traits/pred_engine_trait.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include "omath/projectile_prediction/proj_pred_engine.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"
#include "omath/trigonometry/angles.hpp"
#include <cmath>
#include <concepts>
#include <cstddef>
#include <optional>

namespace omath::projectile_prediction
{
    template<class T, class ArithmeticType>
    concept PredEngineConcept =
            requires(const Projectile<ArithmeticType>& projectile, const Target<ArithmeticType>& target,
                     const Vector3<ArithmeticType>& vec_a, const Vector3<ArithmeticType>& vec_b,
                     Vector3<ArithmeticType> v3, ArithmeticType pitch, ArithmeticType yaw, ArithmeticType time,
                     ArithmeticType gravity, std::optional<ArithmeticType> maybe_pitch) {
                {
                    T::predict_projectile_position(projectile, pitch, yaw, time, gravity)
                } -> std::same_as<Vector3<ArithmeticType>>;
                { T::predict_target_position(target, time, gravity) } -> std::same_as<Vector3<ArithmeticType>>;
                { T::calc_vector_2d_distance(vec_a) } -> std::same_as<ArithmeticType>;
                { T::get_vector_height_coordinate(vec_b) } -> std::same_as<ArithmeticType>;
                { T::calc_viewpoint_from_angles(projectile, v3, maybe_pitch) } -> std::same_as<Vector3<ArithmeticType>>;
                { T::calc_direct_pitch_angle(vec_a, vec_b) } -> std::same_as<ArithmeticType>;
                { T::calc_direct_yaw_angle(vec_a, vec_b) } -> std::same_as<ArithmeticType>;

                requires noexcept(T::predict_projectile_position(projectile, pitch, yaw, time, gravity));
                requires noexcept(T::predict_target_position(target, time, gravity));
                requires noexcept(T::calc_vector_2d_distance(vec_a));
                requires noexcept(T::get_vector_height_coordinate(vec_b));
                requires noexcept(T::calc_viewpoint_from_angles(projectile, v3, maybe_pitch));
                requires noexcept(T::calc_direct_pitch_angle(vec_a, vec_b));
                requires noexcept(T::calc_direct_yaw_angle(vec_a, vec_b));
            };

    template<class EngineTrait = source_engine::PredEngineTrait, class ArithmeticType = float>
    requires PredEngineConcept<EngineTrait, ArithmeticType>
    class ProjPredEngineLegacy final : public ProjPredEngineInterface<ArithmeticType>
    {
    public:
        explicit ProjPredEngineLegacy(const ArithmeticType gravity_constant, const ArithmeticType simulation_time_step,
                                      const ArithmeticType maximum_simulation_time,
                                      const ArithmeticType distance_tolerance) noexcept
            : m_gravity_constant(gravity_constant), m_simulation_time_step(simulation_time_step),
              m_maximum_simulation_time(maximum_simulation_time), m_distance_tolerance(distance_tolerance)
        {
        }

        [[nodiscard]]
        std::optional<Vector3<ArithmeticType>>
        maybe_calculate_aim_point(const Projectile<ArithmeticType>& projectile,
                                  const Target<ArithmeticType>& target) const noexcept override
        {
            const auto solution = find_solution(projectile, target);
            if (!solution)
                return std::nullopt;

            return EngineTrait::calc_viewpoint_from_angles(projectile, solution->predicted_target_position,
                                                           solution->pitch);
        }

        [[nodiscard]]
        std::optional<AimAngles<ArithmeticType>>
        maybe_calculate_aim_angles(const Projectile<ArithmeticType>& projectile,
                                   const Target<ArithmeticType>& target) const noexcept override
        {
            const auto solution = find_solution(projectile, target);
            if (!solution)
                return std::nullopt;

            const auto yaw = EngineTrait::calc_direct_yaw_angle(projectile.m_origin + projectile.m_launch_offset,
                                                                solution->predicted_target_position);
            return AimAngles<ArithmeticType>{solution->pitch, yaw};
        }

    private:
        struct Solution
        {
            Vector3<ArithmeticType> predicted_target_position;
            ArithmeticType pitch;
        };

        // Everything the per-step solve needs that does not depend on time. The scan runs up to
        // m_maximum_simulation_time / m_simulation_time_step steps, so these are worth computing once.
        struct LaunchContext
        {
            Vector3<ArithmeticType> origin;
            ArithmeticType gravity;
            ArithmeticType speed_sqr;
            ArithmeticType speed_pow4;
        };

        [[nodiscard]]
        std::optional<Solution> find_solution(const Projectile<ArithmeticType>& projectile,
                                              const Target<ArithmeticType>& target) const noexcept
        {
            // A non-positive step or horizon has no scan to run. Without this the accumulating loop below would
            // never terminate for a zero step.
            if (!(m_simulation_time_step > ArithmeticType{0}) || !(m_maximum_simulation_time > ArithmeticType{0}))
                return std::nullopt;

            const auto launch_speed_sqr = projectile.m_launch_speed * projectile.m_launch_speed;
            const LaunchContext launch{
                    .origin = projectile.m_origin + projectile.m_launch_offset,
                    .gravity = m_gravity_constant * projectile.m_gravity_scale,
                    .speed_sqr = launch_speed_sqr,
                    .speed_pow4 = launch_speed_sqr * launch_speed_sqr,
            };

            // time = step * index rather than time += step: repeated addition drifts (a 1 ms step over 50 s lands
            // 16 steps and 16 ms off), and the count below is exactly what the parameters say.
            const auto step_count =
                    static_cast<std::size_t>(std::ceil(m_maximum_simulation_time / m_simulation_time_step));

            for (std::size_t step = 0; step < step_count; ++step)
            {
                const auto time = m_simulation_time_step * static_cast<ArithmeticType>(step);
                const auto predicted_target_position =
                        EngineTrait::predict_target_position(target, time, m_gravity_constant);

                const auto projectile_pitch =
                        maybe_calculate_projectile_launch_pitch_angle(launch, predicted_target_position);

                if (!projectile_pitch.has_value()) [[unlikely]]
                    continue;

                if (!is_projectile_reached_target(launch, predicted_target_position, projectile,
                                                  projectile_pitch.value(), time))
                    continue;

                return Solution{predicted_target_position, projectile_pitch.value()};
            }
            return std::nullopt;
        }

        ArithmeticType m_gravity_constant;
        ArithmeticType m_simulation_time_step;
        ArithmeticType m_maximum_simulation_time;
        ArithmeticType m_distance_tolerance;

        // Realization of this formula (low arc branch):
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
        std::optional<ArithmeticType>
        maybe_calculate_projectile_launch_pitch_angle(const LaunchContext& launch,
                                                      const Vector3<ArithmeticType>& target_position) const noexcept
        {
            if (launch.gravity == ArithmeticType{0})
                return EngineTrait::calc_direct_pitch_angle(launch.origin, target_position);

            const auto delta = target_position - launch.origin;

            const auto distance2d = EngineTrait::calc_vector_2d_distance(delta);
            const auto height = EngineTrait::get_vector_height_coordinate(delta);

            // g x^2 + 2 y v^2, shared by the discriminant and the tangent below
            const auto inner = launch.gravity * distance2d * distance2d + ArithmeticType{2} * height * launch.speed_sqr;
            const auto discriminant = launch.speed_pow4 - launch.gravity * inner;

            if (discriminant < ArithmeticType{0}) [[unlikely]]
                return std::nullopt;

            // Straight up or down: the formula divides by g x. The direct angle is the only launch direction anyway.
            if (distance2d == ArithmeticType{0})
                return EngineTrait::calc_direct_pitch_angle(launch.origin, target_position);

            // tan(theta) = (v^2 - sqrt(D)) / (g x) multiplied through by (v^2 + sqrt(D)). For fast projectiles v^2 and
            // sqrt(D) are nearly equal and the plain form cancels most of the float mantissa (about 0.002 degrees of
            // error at 5000 units/s); the conjugate form adds them instead.
            const auto tangent = inner / (distance2d * (launch.speed_sqr + std::sqrt(discriminant)));

            return angles::radians_to_degrees(std::atan(tangent));
        }

        [[nodiscard]]
        bool is_projectile_reached_target(const LaunchContext& launch, const Vector3<ArithmeticType>& target_position,
                                          const Projectile<ArithmeticType>& projectile, const ArithmeticType pitch,
                                          const ArithmeticType time) const noexcept
        {
            const auto yaw = EngineTrait::calc_direct_yaw_angle(launch.origin, target_position);
            const auto projectile_position =
                    EngineTrait::predict_projectile_position(projectile, pitch, yaw, time, m_gravity_constant);

            // Squared compare keeps the per step square root out of the scan
            return projectile_position.distance_to_sqr(target_position) <= m_distance_tolerance * m_distance_tolerance;
        }
    };
} // namespace omath::projectile_prediction
