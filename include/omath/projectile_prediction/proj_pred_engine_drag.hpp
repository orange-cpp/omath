//
// Created by Vlad on 9/19/2026.
//

#pragma once
#include "omath/engines/source_engine/traits/pred_engine_trait.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include "omath/projectile_prediction/launcher.hpp"
#include "omath/projectile_prediction/pred_engine_concept.hpp"
#include "omath/projectile_prediction/proj_pred_engine.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/projectile_flight.hpp"
#include "omath/projectile_prediction/target.hpp"
#include "omath/trigonometry/angles.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <utility>

namespace omath::projectile_prediction
{
    // Aim solver for rounds that lose speed to the air. The closed-form engines pick their launch pitch from a
    // parabola, and there is no closed form once drag is involved: the pipe a parabola promises at 1000 units arrives
    // about 150 short. This engine finds the pitch by flying the round through ProjectileFlight, so whatever it
    // returns is a shot that flight lands.
    //
    // It answers ProjPredEngineInterface and fills AimSolution the same way ProjPredEngineLegacy does, so the two are
    // interchangeable to everything downstream. A round without drag is better served by the closed-form engines,
    // which are exact for it and do not depend on the game's physics step.
    template<class EngineTrait = source_engine::PredEngineTrait, class ArithmeticType = float>
    requires PredEngineConcept<EngineTrait, ArithmeticType>
    class ProjPredEngineDrag final : public ProjPredEngineInterface<ArithmeticType>
    {
    public:
        // simulation_time_step is the physics step of the game being predicted (Source: its tick interval), not an
        // accuracy setting. See ProjectileFlight.
        explicit ProjPredEngineDrag(const ArithmeticType gravity_constant, const ArithmeticType simulation_time_step,
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
            constexpr int max_iterations = 12;

            if (!(m_simulation_time_step > ArithmeticType{0}) || !(m_maximum_simulation_time > ArithmeticType{0})
                || !(projectile.m_launch_speed > ArithmeticType{0}))
                return std::nullopt;

            // Probe flights may run past the horizon, so that a shot right at it still has a pass on either side to
            // close in from. The horizon itself is enforced on the answer.
            const auto time_limit =
                    m_maximum_simulation_time * static_cast<ArithmeticType>(1.5) + static_cast<ArithmeticType>(0.5);

            // Where the target will be depends on how long the round takes, and how long the round takes depends on
            // where the target will be. Every pass of this loop settles both a little further, and because nothing
            // on foot outruns a projectile it closes in fast: the error shrinks by about the ratio of the two speeds
            // each time.
            auto time = launcher.eye_origin.distance_to(target.m_origin) / projectile.m_launch_speed;
            auto yaw = EngineTrait::calc_direct_yaw_angle(launcher.eye_origin, target.m_origin);

            std::optional<ArithmeticType> view_pitch;
            auto predicted = target.m_origin;

            for (int iteration = 0; iteration < max_iterations; ++iteration)
            {
                predicted = EngineTrait::predict_target_position(target, time, m_gravity_constant);

                if (!view_pitch)
                {
                    const auto start =
                            maybe_calculate_parabola_launch_pitch(projectile, launcher.eye_origin, predicted);
                    if (!start)
                        return std::nullopt;

                    view_pitch = *start - launcher.launch_pitch_offset;
                }

                // The muzzle sits off to one side and swings with the yaw, so the yaw that points the muzzle's own
                // flight plane at the target is found from where the muzzle ends up
                for (int pass = 0; pass < 2; ++pass)
                    yaw = EngineTrait::calc_direct_yaw_angle(
                            launcher.launch_origin(EngineTrait::calc_view_basis(*view_pitch, yaw)), predicted);

                // After the first pass the target has barely moved, so the search starts tight
                const auto first_step = static_cast<ArithmeticType>(iteration == 0 ? 0.5 : 0.125);
                const auto solved =
                        solve_view_pitch(projectile, launcher, yaw, predicted, *view_pitch, first_step, time_limit);
                if (!solved)
                    return std::nullopt;

                // The round gets to `predicted` at solved->time, by when the target has moved on to where it will be
                // at that time. Once those two are the same place, the shot is the answer.
                const auto drift = EngineTrait::predict_target_position(target, solved->time, m_gravity_constant)
                                           .distance_to(predicted);

                view_pitch = solved->view_pitch;
                time = solved->time;

                if (drift <= m_distance_tolerance / ArithmeticType{64})
                    break;
            }

            if (time > m_maximum_simulation_time || !is_view_pitch_reachable(*view_pitch, yaw))
                return std::nullopt;

            // Whatever came out above has to put the round on the target, at the time claimed
            predicted = EngineTrait::predict_target_position(target, time, m_gravity_constant);

            const AimAngles<ArithmeticType> angles{*view_pitch, yaw};
            if (predict_projectile_position(projectile, launcher, angles, time).distance_to(predicted)
                > m_distance_tolerance)
                return std::nullopt;

            // Every point on the eye's aim ray lands on the same pixel; the target's distance keeps the point at a
            // sensible depth for anything that uses it in 3D.
            const auto forward = EngineTrait::calc_view_basis(angles.pitch, angles.yaw).forward;

            return AimSolution<ArithmeticType>{
                    .angles = angles,
                    .aim_point = launcher.eye_origin + forward * launcher.eye_origin.distance_to(predicted),
                    .predicted_target_position = predicted,
                    .time_of_flight = time,
            };
        }

        // Where a round fired along view_angles is after `time` seconds. Between steps it is on the straight line
        // joining them, which is how the game moves and draws it too.
        [[nodiscard]]
        Vector3<ArithmeticType> predict_projectile_position(const Projectile<ArithmeticType>& projectile,
                                                            const Launcher<ArithmeticType>& launcher,
                                                            const AimAngles<ArithmeticType>& view_angles,
                                                            const ArithmeticType time) const noexcept
        {
            Flight flight{projectile, launcher, view_angles, m_gravity_constant, m_simulation_time_step};

            const auto steps = std::max(ArithmeticType{0}, time) / m_simulation_time_step;
            const auto whole_steps = static_cast<std::size_t>(steps);

            for (std::size_t step = 0; step < whole_steps; ++step)
                flight.step();

            const auto before = flight.position();
            flight.step();

            return before + (flight.position() - before) * (steps - static_cast<ArithmeticType>(whole_steps));
        }

    private:
        using Flight = ProjectileFlight<EngineTrait, ArithmeticType>;

        struct Pass
        {
            // How far above the target point the round is as it goes by it, negative when it is below
            ArithmeticType height_error;
            ArithmeticType time;
        };

        struct PitchSolution
        {
            ArithmeticType view_pitch;
            ArithmeticType time;
        };

        // Flies the round from this view pitch until it has covered the horizontal distance to the target point, and
        // reports the height it passes at and when. The muzzle is placed from the same angles, so the way it swings
        // with the view is part of the answer rather than an error in it. Empty when the round never gets that far.
        [[nodiscard]]
        std::optional<Pass>
        fly_past(const Projectile<ArithmeticType>& projectile, const Launcher<ArithmeticType>& launcher,
                 const ArithmeticType view_pitch, const ArithmeticType yaw, const Vector3<ArithmeticType>& heading,
                 const Vector3<ArithmeticType>& target_point, const ArithmeticType time_limit) const noexcept
        {
            Flight flight{projectile, launcher, {view_pitch, yaw}, m_gravity_constant, m_simulation_time_step};

            const auto to_target = target_point - flight.origin();
            const auto distance = to_target.dot(heading);

            if (!(distance > ArithmeticType{0}))
                return std::nullopt;

            const auto height = EngineTrait::get_vector_height_coordinate(to_target);

            ArithmeticType previous_travel{0};
            ArithmeticType previous_height{0};

            while (flight.time() < time_limit)
            {
                const auto previous_time = flight.time();
                flight.step();

                const auto flown = flight.position() - flight.origin();
                const auto travel = flown.dot(heading);
                const auto flown_height = EngineTrait::get_vector_height_coordinate(flown);

                if (travel >= distance)
                {
                    // The round moves in a straight line within a step, so the crossing is linear too
                    const auto fraction = (distance - previous_travel) / (travel - previous_travel);

                    return Pass{previous_height + (flown_height - previous_height) * fraction - height,
                                previous_time + m_simulation_time_step * fraction};
                }

                previous_travel = travel;
                previous_height = flown_height;
            }

            return std::nullopt;
        }

        // The low-arc pitch a round without drag would need, which is the formula ProjPredEngineLegacy solves with.
        // With drag the real answer is always a little above it, which makes it the place to start looking from.
        // Empty when even a parabola falls short, and drag only ever shortens the reach.
        [[nodiscard]]
        std::optional<ArithmeticType>
        maybe_calculate_parabola_launch_pitch(const Projectile<ArithmeticType>& projectile,
                                              const Vector3<ArithmeticType>& origin,
                                              const Vector3<ArithmeticType>& target_point) const noexcept
        {
            const auto gravity = m_gravity_constant * projectile.m_gravity_scale;
            const auto delta = target_point - origin;
            const auto distance2d = EngineTrait::calc_vector_2d_distance(delta);

            if (gravity == ArithmeticType{0} || distance2d == ArithmeticType{0})
                return EngineTrait::calc_direct_pitch_angle(origin, target_point);

            const auto speed_sqr = projectile.m_launch_speed * projectile.m_launch_speed;
            const auto inner = gravity * distance2d * distance2d
                               + ArithmeticType{2} * EngineTrait::get_vector_height_coordinate(delta) * speed_sqr;
            const auto discriminant = speed_sqr * speed_sqr - gravity * inner;

            if (discriminant < ArithmeticType{0})
                return std::nullopt;

            return angles::radians_to_degrees(std::atan(inner / (distance2d * (speed_sqr + std::sqrt(discriminant)))));
        }

        // Finds the low-arc view pitch that flies the round through target_point. It walks away from start_pitch in
        // whichever direction the first miss says to, until the round passes on the other side of the target, then
        // closes in on the crossing between the two.
        [[nodiscard]]
        std::optional<PitchSolution>
        solve_view_pitch(const Projectile<ArithmeticType>& projectile, const Launcher<ArithmeticType>& launcher,
                         const ArithmeticType yaw, const Vector3<ArithmeticType>& target_point,
                         const ArithmeticType start_pitch, const ArithmeticType first_step,
                         const ArithmeticType time_limit) const noexcept
        {
            constexpr auto pitch_limit = static_cast<ArithmeticType>(90);
            constexpr auto max_step = static_cast<ArithmeticType>(4);
            constexpr int max_refinements = 24;

            // How close to the target's height a pass has to come before the pitch counts as found
            const auto height_tolerance = m_distance_tolerance / ArithmeticType{100};

            // A level view looks along the horizontal, whichever axes the engine keeps that in
            const auto heading = EngineTrait::calc_view_basis(ArithmeticType{0}, yaw).forward;

            const auto probe = [&](const ArithmeticType pitch)
            {
                return fly_past(projectile, launcher, pitch, yaw, heading, target_point, time_limit);
            };

            auto near_pitch = std::clamp(start_pitch, -pitch_limit, pitch_limit);
            auto near = probe(near_pitch);

            if (!near)
                return std::nullopt;
            if (std::abs(near->height_error) <= height_tolerance)
                return PitchSolution{near_pitch, near->time};

            const auto direction = near->height_error < ArithmeticType{0} ? ArithmeticType{1} : ArithmeticType{-1};

            ArithmeticType far_pitch{};
            std::optional<Pass> far;

            for (auto step = first_step;; step = std::min(step * ArithmeticType{2}, max_step))
            {
                if (near_pitch * direction >= pitch_limit)
                    return std::nullopt;

                far_pitch = std::clamp(near_pitch + direction * step, -pitch_limit, pitch_limit);
                far = probe(far_pitch);

                // A lob this steep that still cannot cover the distance in time is over the top of what the weapon
                // can reach
                if (!far)
                    return std::nullopt;
                if (std::abs(far->height_error) <= height_tolerance)
                    return PitchSolution{far_pitch, far->time};

                if ((far->height_error < ArithmeticType{0}) != (near->height_error < ArithmeticType{0}))
                    break;

                // Raising the pitch has to bring the round up at the target. Once it stops doing that the arc has
                // gone over its peak without ever getting there: out of reach.
                if (direction > ArithmeticType{0} && far->height_error <= near->height_error)
                    return std::nullopt;

                near_pitch = far_pitch;
                near = far;
            }

            // Regula falsi with the Illinois correction, between the pass below and the pass above
            auto below_pitch = near_pitch;
            auto below_error = near->height_error;
            auto above_pitch = far_pitch;
            auto above_error = far->height_error;

            if (below_error > ArithmeticType{0})
            {
                std::swap(below_pitch, above_pitch);
                std::swap(below_error, above_error);
            }

            PitchSolution best{below_pitch, near->time};
            int last_side = 0;

            for (int i = 0; i < max_refinements; ++i)
            {
                const auto pitch =
                        (below_pitch * above_error - above_pitch * below_error) / (above_error - below_error);
                const auto pass = probe(pitch);

                if (!pass)
                    return std::nullopt;

                best = {pitch, pass->time};

                if (std::abs(pass->height_error) <= height_tolerance
                    || std::abs(above_pitch - below_pitch) < static_cast<ArithmeticType>(1e-4))
                    break;

                if (pass->height_error < ArithmeticType{0})
                {
                    below_pitch = pitch;
                    below_error = pass->height_error;
                    if (last_side == -1)
                        above_error /= ArithmeticType{2};
                    last_side = -1;
                }
                else
                {
                    above_pitch = pitch;
                    above_error = pass->height_error;
                    if (last_side == 1)
                        below_error /= ArithmeticType{2};
                    last_side = 1;
                }
            }

            return best;
        }

        // calc_view_basis() clamps the way the engine does, so a forward vector that no longer has the requested
        // pitch means nobody can set this shot up. Same check ProjPredEngineLegacy makes.
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
    };
} // namespace omath::projectile_prediction
