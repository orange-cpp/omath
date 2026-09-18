//
// Created by Vlad on 6/9/2024.
//

#pragma once

#include "omath/engines/source_engine/traits/pred_engine_trait.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include "omath/projectile_prediction/launcher.hpp"
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
                     const Vector3<ArithmeticType>& vec_a, const Vector3<ArithmeticType>& vec_b, ArithmeticType pitch,
                     ArithmeticType yaw, ArithmeticType time, ArithmeticType gravity) {
                {
                    T::predict_projectile_position(vec_a, projectile, pitch, yaw, time, gravity)
                } -> std::same_as<Vector3<ArithmeticType>>;
                { T::predict_target_position(target, time, gravity) } -> std::same_as<Vector3<ArithmeticType>>;
                { T::calc_vector_2d_distance(vec_a) } -> std::same_as<ArithmeticType>;
                { T::get_vector_height_coordinate(vec_b) } -> std::same_as<ArithmeticType>;
                { T::calc_view_basis(pitch, yaw) } -> std::same_as<ViewBasis<ArithmeticType>>;
                { T::calc_direct_pitch_angle(vec_a, vec_b) } -> std::same_as<ArithmeticType>;
                { T::calc_direct_yaw_angle(vec_a, vec_b) } -> std::same_as<ArithmeticType>;

                requires noexcept(T::predict_projectile_position(vec_a, projectile, pitch, yaw, time, gravity));
                requires noexcept(T::predict_target_position(target, time, gravity));
                requires noexcept(T::calc_vector_2d_distance(vec_a));
                requires noexcept(T::get_vector_height_coordinate(vec_b));
                requires noexcept(T::calc_view_basis(pitch, yaw));
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
        std::optional<AimSolution<ArithmeticType>>
        maybe_calculate_aim(const Projectile<ArithmeticType>& projectile, const Launcher<ArithmeticType>& launcher,
                            const Target<ArithmeticType>& target) const noexcept override
        {
            const auto solution = find_solution(projectile, launcher, target);
            if (!solution)
                return std::nullopt;

            // Every point on the eye's aim ray lands on the same pixel; the target's distance keeps the point at a
            // sensible depth for anything that uses it in 3D.
            const auto forward = EngineTrait::calc_view_basis(solution->pitch, solution->yaw).forward;
            const auto distance = launcher.eye_origin.distance_to(solution->predicted_target_position);

            return AimSolution<ArithmeticType>{
                    .angles = {solution->pitch, solution->yaw},
                    .aim_point = launcher.eye_origin + forward * distance,
                    .predicted_target_position = solution->predicted_target_position,
                    .time_of_flight = solution->time,
            };
        }

    private:
        struct Solution
        {
            Vector3<ArithmeticType> predicted_target_position;
            // View angles: the launch pitch with the launcher's pitch offset already taken off
            ArithmeticType pitch;
            ArithmeticType yaw;
            ArithmeticType time;
        };

        // Everything the per-step solve needs that does not depend on time. The scan runs up to
        // m_maximum_simulation_time / m_simulation_time_step steps, so these are worth computing once.
        struct LaunchContext
        {
            Launcher<ArithmeticType> launcher;
            // eye + world offset: the launch origin whenever the muzzle offset is zero
            Vector3<ArithmeticType> fixed_origin;
            ArithmeticType gravity;
            ArithmeticType speed_sqr;
            ArithmeticType speed_pow4;
        };

        [[nodiscard]]
        std::optional<Solution> find_solution(const Projectile<ArithmeticType>& projectile,
                                              const Launcher<ArithmeticType>& launcher,
                                              const Target<ArithmeticType>& target) const noexcept
        {
            // A non-positive step or horizon has no scan to run. Without this an accumulating loop would never
            // terminate for a zero step.
            if (!(m_simulation_time_step > ArithmeticType{0}) || !(m_maximum_simulation_time > ArithmeticType{0}))
                return std::nullopt;

            const auto launch_speed_sqr = projectile.m_launch_speed * projectile.m_launch_speed;
            const LaunchContext launch{
                    .launcher = launcher,
                    .fixed_origin = launcher.eye_origin + launcher.world_offset,
                    .gravity = m_gravity_constant * projectile.m_gravity_scale,
                    .speed_sqr = launch_speed_sqr,
                    .speed_pow4 = launch_speed_sqr * launch_speed_sqr,
            };

            // Two instantiations so the common fixed-muzzle case keeps the tight loop it had before Launcher existed:
            // no muzzle placement, and a pitch solve small enough to stay inlined in the scan.
            if (launcher.muzzle_offset.is_zero())
                return scan<false>(launch, projectile, target);
            return scan<true>(launch, projectile, target);
        }

        // With a view-relative muzzle offset the launch origin depends on the very angles being solved for, so the
        // muzzle is first placed from the direct angles to the target and the solve is repeated once from where it
        // ends up. One pass is enough: the offset is a few units against a target hundreds away, so the second
        // correction is far below the step tolerance.
        //
        // `pitch` below is always the launch pitch, the direction the round leaves in. The muzzle turns with the view,
        // not with the round, so every basis is built from the view pitch: launch pitch minus the launcher's offset.
        template<bool MuzzleRotates>
        [[nodiscard]]
        std::optional<Solution> scan(const LaunchContext& launch, const Projectile<ArithmeticType>& projectile,
                                     const Target<ArithmeticType>& target) const noexcept
        {
            // time = step * index rather than time += step: repeated addition drifts (a 1 ms step over 50 s lands
            // 16 steps and 16 ms off), and the count below is exactly what the parameters say.
            const auto step_count =
                    static_cast<std::size_t>(std::ceil(m_maximum_simulation_time / m_simulation_time_step));
            const auto pitch_offset = launch.launcher.launch_pitch_offset;

            for (std::size_t step = 0; step < step_count; ++step)
            {
                const auto time = m_simulation_time_step * static_cast<ArithmeticType>(step);
                const auto predicted_target_position =
                        EngineTrait::predict_target_position(target, time, m_gravity_constant);

                auto origin = launch.fixed_origin;
                if constexpr (MuzzleRotates)
                    origin = launch.launcher.launch_origin(EngineTrait::calc_view_basis(
                            EngineTrait::calc_direct_pitch_angle(launch.launcher.eye_origin, predicted_target_position)
                                    - pitch_offset,
                            EngineTrait::calc_direct_yaw_angle(launch.launcher.eye_origin, predicted_target_position)));

                auto pitch = maybe_calculate_projectile_launch_pitch_angle(launch, origin, predicted_target_position);

                if (!pitch.has_value()) [[unlikely]]
                    continue;

                auto yaw = EngineTrait::calc_direct_yaw_angle(origin, predicted_target_position);

                if constexpr (MuzzleRotates)
                {
                    origin = launch.launcher.launch_origin(EngineTrait::calc_view_basis(*pitch - pitch_offset, yaw));

                    pitch = maybe_calculate_projectile_launch_pitch_angle(launch, origin, predicted_target_position);
                    if (!pitch.has_value())
                        continue;

                    yaw = EngineTrait::calc_direct_yaw_angle(origin, predicted_target_position);
                }

                if (!is_projectile_reached_target(origin, predicted_target_position, projectile, *pitch, yaw, time))
                    continue;

                const auto view_pitch = *pitch - pitch_offset;
                if (pitch_offset != ArithmeticType{0} && !is_view_pitch_reachable(view_pitch, yaw))
                    continue;

                return Solution{predicted_target_position, view_pitch, yaw, time};
            }
            return std::nullopt;
        }

        // A pitch offset can put the view pitch that produces the launch outside what the engine lets a player look
        // at (Source stops at +-89 degrees). calc_view_basis() clamps the way the engine does, so a forward vector
        // that no longer has the requested pitch means nobody can set this shot up. Without an offset the launch pitch
        // is the view pitch, and the reach check above already flies it through the same clamp.
        [[nodiscard]]
        static bool is_view_pitch_reachable(const ArithmeticType view_pitch, const ArithmeticType yaw) noexcept
        {
            constexpr auto tolerance_degrees = static_cast<ArithmeticType>(0.1);

            const auto forward = EngineTrait::calc_view_basis(view_pitch, yaw).forward;
            const auto reachable_pitch = EngineTrait::calc_direct_pitch_angle(Vector3<ArithmeticType>{}, forward);

            return std::abs(reachable_pitch - view_pitch) <= tolerance_degrees;
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
                                                      const Vector3<ArithmeticType>& launch_origin,
                                                      const Vector3<ArithmeticType>& target_position) const noexcept
        {
            if (launch.gravity == ArithmeticType{0})
                return EngineTrait::calc_direct_pitch_angle(launch_origin, target_position);

            const auto delta = target_position - launch_origin;

            const auto distance2d = EngineTrait::calc_vector_2d_distance(delta);
            const auto height = EngineTrait::get_vector_height_coordinate(delta);

            // g x^2 + 2 y v^2, shared by the discriminant and the tangent below
            const auto inner = launch.gravity * distance2d * distance2d + ArithmeticType{2} * height * launch.speed_sqr;
            const auto discriminant = launch.speed_pow4 - launch.gravity * inner;

            if (discriminant < ArithmeticType{0}) [[unlikely]]
                return std::nullopt;

            // Straight up or down: the formula divides by g x. The direct angle is the only launch direction anyway.
            if (distance2d == ArithmeticType{0})
                return EngineTrait::calc_direct_pitch_angle(launch_origin, target_position);

            // tan(theta) = (v^2 - sqrt(D)) / (g x) multiplied through by (v^2 + sqrt(D)). For fast projectiles v^2 and
            // sqrt(D) are nearly equal and the plain form cancels most of the float mantissa (about 0.002 degrees of
            // error at 5000 units/s); the conjugate form adds them instead.
            const auto tangent = inner / (distance2d * (launch.speed_sqr + std::sqrt(discriminant)));

            return angles::radians_to_degrees(std::atan(tangent));
        }

        [[nodiscard]]
        bool is_projectile_reached_target(const Vector3<ArithmeticType>& launch_origin,
                                          const Vector3<ArithmeticType>& target_position,
                                          const Projectile<ArithmeticType>& projectile, const ArithmeticType pitch,
                                          const ArithmeticType yaw, const ArithmeticType time) const noexcept
        {
            const auto projectile_position = EngineTrait::predict_projectile_position(launch_origin, projectile, pitch,
                                                                                      yaw, time, m_gravity_constant);

            // Squared compare keeps the per step square root out of the scan
            return projectile_position.distance_to_sqr(target_position) <= m_distance_tolerance * m_distance_tolerance;
        }
    };
} // namespace omath::projectile_prediction
