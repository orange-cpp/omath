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
    // interchangeable to everything downstream.
    //
    // It is not only for drag. A round with no gravity flies a straight line, which stepping follows exactly at any
    // step, and this engine finds its answer by closing in on it rather than by hoping a scan step lands inside the
    // tolerance, so it cannot come back empty for a target that closes faster than the scan allowed for. What it is
    // wrong for is a round with gravity and no drag in a game that moves such rounds on a true parabola: the stepped
    // fall here drops a little further than that (see ProjectileFlight), and the closed-form engines are exact.
    //
    // Built for Arc::HIGH it solves for the lob instead: the steeper of the two pitches that reach the target, which
    // drops onto it from above and takes longer to get there. Such a shot is refused, like any other, when its flight
    // would outlast maximum_simulation_time, and a lob to anything near takes a long time.
    template<class EngineTrait = source_engine::PredEngineTrait, class ArithmeticType = float>
    requires PredEngineConcept<EngineTrait, ArithmeticType>
    class ProjPredEngineDrag final : public ProjPredEngineInterface<ArithmeticType>
    {
    public:
        // simulation_time_step is the physics step of the game being predicted (Source: its tick interval), not an
        // accuracy setting. See ProjectileFlight.
        explicit ProjPredEngineDrag(const ArithmeticType gravity_constant, const ArithmeticType simulation_time_step,
                                    const ArithmeticType maximum_simulation_time,
                                    const ArithmeticType distance_tolerance, const Arc arc = Arc::LOW) noexcept
            : m_gravity_constant(gravity_constant), m_simulation_time_step(simulation_time_step),
              m_maximum_simulation_time(maximum_simulation_time), m_distance_tolerance(distance_tolerance), m_arc(arc)
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
            // close in from: the pass above is at most four degrees steeper than the answer, which gets it there a
            // tenth or so later. Any further than that only makes a flight that was never going to arrive take
            // longer to give up. The horizon itself is enforced on the answer.
            const auto time_limit = m_maximum_simulation_time * static_cast<ArithmeticType>(1.25);

            // A round without gravity flies one straight line, and both arcs are it
            const auto arc = m_gravity_constant * projectile.m_gravity_scale > ArithmeticType{0} ? m_arc : Arc::LOW;

            // Where the target will be depends on how long the round takes, and how long the round takes depends on
            // where the target will be. The time that satisfies both is the one where the round, aimed at where the
            // target is at that time, takes exactly that long to get there: mismatch(time) = arrival - time = 0.
            //
            // Feeding each arrival back in as the next guess would find it only while the target is slow. That
            // closes in by the ratio of the target's speed along the line of fire to the round's every pass, which
            // is a crawl for a target blown away at two thirds of the round's speed and gets further out every pass
            // for one blown towards the shooter faster than the round flies. So the first step is that guess, and
            // every one after it is a secant step through the last two mismatches, which does not care.
            auto time = launcher.eye_origin.distance_to(target.m_origin) / projectile.m_launch_speed;
            auto yaw = EngineTrait::calc_direct_yaw_angle(launcher.eye_origin, target.m_origin);

            auto predicted = target.m_origin;

            // How far above the parabola's pitch the last pass ended up. Drag asks for about the same extra loft from
            // one pass to the next, which makes it a good guess at where the next search will end.
            std::optional<ArithmeticType> loft;

            // One shot at the target as it will be at `at`: the view pitch that lands there, and when the round does.
            // The search is always anchored to what a parabola would need for that same point, on the arc being
            // solved for. Carrying the last pass's pitch over instead looks cheaper and is a trap: a step in time can
            // move the target by tens of degrees, and a search started on the wrong side of the arc's peak, where
            // turning the pitch the way it expects moves the round the other way, walks itself into the limit.
            const auto shoot_at = [&](const ArithmeticType at) -> std::optional<PitchSolution>
            {
                predicted = EngineTrait::predict_target_position(target, at, m_gravity_constant);

                const auto parabola =
                        maybe_calculate_parabola_launch_pitches(projectile, launcher.eye_origin, predicted);
                if (!parabola)
                    return std::nullopt;

                auto start_pitch = (arc == Arc::LOW ? parabola->low : parabola->high) - launcher.launch_pitch_offset;

                // The high arc lies between the two parabola pitches: drag needs more loft than the low one and
                // stands less than the high one. And no lob steeper than the one whose parabola takes the whole
                // time limit to cover the distance gets there in time, since drag only ever makes a flight later,
                // so the search need not start above that either.
                const auto floor_pitch = parabola->low - launcher.launch_pitch_offset;
                if (arc == Arc::HIGH)
                {
                    const auto cosine = EngineTrait::calc_vector_2d_distance(predicted - launcher.eye_origin)
                                        / (projectile.m_launch_speed * time_limit);
                    if (cosine < ArithmeticType{1})
                        start_pitch = std::min(start_pitch, angles::radians_to_degrees(std::acos(cosine))
                                                                    - launcher.launch_pitch_offset);
                }

                // The muzzle sits off to one side and swings with the yaw, so the yaw that points the muzzle's own
                // flight plane at the target is found from where the muzzle ends up
                for (int pass = 0; pass < 2; ++pass)
                    yaw = EngineTrait::calc_direct_yaw_angle(
                            launcher.launch_origin(EngineTrait::calc_view_basis(start_pitch, yaw)), predicted);

                // A shade past the last loft, on the side that has the round pass above the target, so that the
                // pair of passes closes on the answer straight away
                std::optional<ArithmeticType> hint;
                if (loft)
                    hint = start_pitch + *loft * static_cast<ArithmeticType>(1.05)
                           + static_cast<ArithmeticType>(0.1) * climb(arc);

                const auto shot = solve_view_pitch(projectile, launcher, yaw, predicted, start_pitch, floor_pitch, hint,
                                                   time_limit, arc);
                if (shot)
                    loft = shot->view_pitch - start_pitch;

                return shot;
            };

            auto solved = shoot_at(time);

            if (!solved)
            {
                // The first guess can fall on a time at which the target cannot be reached although it can be later:
                // somebody running in from out of range, a jumper coming down into it. A shot that fails has failed
                // for being too far, so the one other time worth a search is the one along the horizon that brings
                // the target nearest, and only if that is nearer than it was.
                //
                // For a target on the ground only later times count. The guess is how long a straight flight at
                // launch speed takes, and no round gets anywhere sooner than that, so a target that is only nearer
                // earlier on is one that is leaving, and earlier is when it cannot be caught. Looking there cost four
                // more searches to refuse a shot that the first one had already settled.
                //
                // Not so for one in the air. It is falling, and the lower it gets the sooner it is reached, however
                // far off that is, so how near it is says little about whether it can be hit and every time is
                // looked at. Twice a short cut here that reasoned from distance turned down shots at airborne
                // targets that were there to be made.
                constexpr int candidates = 8;

                auto nearest = launcher.eye_origin.distance_to(predicted);
                std::optional<ArithmeticType> nearest_time;

                for (int candidate = 1; candidate <= candidates; ++candidate)
                {
                    const auto at = m_maximum_simulation_time * static_cast<ArithmeticType>(candidate)
                                    / static_cast<ArithmeticType>(candidates);
                    if (!target.m_is_airborne && at <= time)
                        continue;

                    const auto distance = launcher.eye_origin.distance_to(
                            EngineTrait::predict_target_position(target, at, m_gravity_constant));

                    if (distance < nearest)
                    {
                        nearest = distance;
                        nearest_time = at;
                    }
                }

                if (!nearest_time)
                    return std::nullopt;

                time = *nearest_time;
                solved = shoot_at(time);

                if (!solved)
                    return std::nullopt;
            }

            std::optional<ArithmeticType> previous_time;
            ArithmeticType previous_mismatch{};

            for (int iteration = 0; iteration < max_iterations; ++iteration)
            {
                // The round gets to `predicted` at solved->time, by when the target has moved on to where it will be
                // at that time. Once those two are the same place, the shot is the answer.
                const auto settled = EngineTrait::predict_target_position(target, solved->time, m_gravity_constant)
                                             .distance_to(predicted)
                                     <= m_distance_tolerance / ArithmeticType{64};

                const auto mismatch = solved->time - time;
                auto next_time = solved->time;

                if (!settled && previous_time && mismatch != previous_mismatch)
                    next_time = time - mismatch * (time - *previous_time) / (mismatch - previous_mismatch);

                previous_time = time;
                previous_mismatch = mismatch;
                time = std::clamp(next_time, ArithmeticType{0}, time_limit);

                if (settled)
                    break;

                auto next = shoot_at(time);

                // A step can land on a time at which the target is somewhere the round cannot be put at all, although
                // the answer is somewhere it can. Back off towards the last time that could be shot at.
                for (int retry = 0; !next && retry < 2; ++retry)
                {
                    time = (time + *previous_time) / ArithmeticType{2};
                    next = shoot_at(time);
                }

                if (!next)
                    return std::nullopt;

                solved = next;
            }

            if (time > m_maximum_simulation_time || !is_view_pitch_reachable(solved->view_pitch, yaw))
                return std::nullopt;

            // Whatever came out above has to put the round on the target, at the time claimed
            predicted = EngineTrait::predict_target_position(target, time, m_gravity_constant);

            const AimAngles<ArithmeticType> angles{solved->view_pitch, yaw};
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

        struct ParabolaPitches
        {
            ArithmeticType low;
            ArithmeticType high;
        };

        // The two pitches a round without drag would need, the low one being the formula ProjPredEngineLegacy solves
        // with. With drag the real answers lie between them, each a little in from its own side, which makes them
        // the places to start looking from. Empty when even a parabola falls short, and drag only ever shortens the
        // reach.
        [[nodiscard]]
        std::optional<ParabolaPitches>
        maybe_calculate_parabola_launch_pitches(const Projectile<ArithmeticType>& projectile,
                                                const Vector3<ArithmeticType>& origin,
                                                const Vector3<ArithmeticType>& target_point) const noexcept
        {
            const auto gravity = m_gravity_constant * projectile.m_gravity_scale;
            const auto delta = target_point - origin;
            const auto distance2d = EngineTrait::calc_vector_2d_distance(delta);

            if (gravity == ArithmeticType{0} || distance2d == ArithmeticType{0})
            {
                const auto direct = EngineTrait::calc_direct_pitch_angle(origin, target_point);
                return ParabolaPitches{direct, direct};
            }

            const auto speed_sqr = projectile.m_launch_speed * projectile.m_launch_speed;
            const auto inner = gravity * distance2d * distance2d
                               + ArithmeticType{2} * EngineTrait::get_vector_height_coordinate(delta) * speed_sqr;
            const auto discriminant = speed_sqr * speed_sqr - gravity * inner;

            if (discriminant < ArithmeticType{0})
                return std::nullopt;

            const auto root = std::sqrt(discriminant);

            return ParabolaPitches{
                    angles::radians_to_degrees(std::atan(inner / (distance2d * (speed_sqr + root)))),
                    angles::radians_to_degrees(std::atan((speed_sqr + root) / (gravity * distance2d))),
            };
        }

        // Which way to turn the pitch to bring the round up at the target: steeper on the low arc, and the other way
        // about on the high arc, where a steeper lob comes down sooner.
        [[nodiscard]]
        static constexpr ArithmeticType climb(const Arc arc) noexcept
        {
            return arc == Arc::LOW ? ArithmeticType{1} : ArithmeticType{-1};
        }

        // Finds the view pitch on the engine's arc that flies the round through target_point. It walks away from
        // start_pitch in whichever direction the first miss says to, until the round passes on the other side of the
        // target, then closes in on the crossing between the two.
        //
        // start_pitch has to be on the same arc as the answer, which the parabola's pitch for that arc always is,
        // because everything the search concludes it concludes relative to that pass. hint_pitch is a guess at the
        // answer and is only ever used to pair with it: if the round passes on the other side of the target from
        // there the walk is skipped, and if it does not the hint is dropped. A wrong hint costs one flight and can
        // never turn a shot down. floor_pitch is the low arc's parabola pitch, below which a high arc cannot lie.
        [[nodiscard]]
        std::optional<PitchSolution>
        solve_view_pitch(const Projectile<ArithmeticType>& projectile, const Launcher<ArithmeticType>& launcher,
                         const ArithmeticType yaw, const Vector3<ArithmeticType>& target_point,
                         const ArithmeticType start_pitch, const ArithmeticType floor_pitch,
                         const std::optional<ArithmeticType> hint_pitch, const ArithmeticType time_limit,
                         const Arc arc) const noexcept
        {
            constexpr auto pitch_limit = static_cast<ArithmeticType>(90);
            constexpr auto first_step = static_cast<ArithmeticType>(0.5);
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

            const auto lowest = arc == Arc::LOW ? -pitch_limit : std::max(-pitch_limit, floor_pitch);

            auto near_pitch = std::clamp(start_pitch, lowest, pitch_limit);
            auto near = probe(near_pitch);

            if (arc == Arc::HIGH)
            {
                // The parabola's high arc to a near target is all but vertical, and with drag on top of that such a
                // lob takes longer than any flight is allowed. Come down until one gets there. If the first that
                // does is already over the target, the crossing lies steeper still, among the flights too long to
                // count.
                const auto started_at = near_pitch;
                while (!near && near_pitch > lowest)
                {
                    near_pitch = std::max(near_pitch - max_step, lowest);
                    near = probe(near_pitch);
                }
                if (near && near_pitch != started_at && near->height_error > height_tolerance)
                    return std::nullopt;
            }

            if (!near)
                return std::nullopt;
            if (std::abs(near->height_error) <= height_tolerance)
                return PitchSolution{near_pitch, near->time};

            const auto direction = near->height_error < ArithmeticType{0} ? climb(arc) : -climb(arc);

            ArithmeticType far_pitch{};
            std::optional<Pass> far;

            if (hint_pitch)
            {
                far_pitch = std::clamp(*hint_pitch, lowest, pitch_limit);
                far = probe(far_pitch);

                if (far && std::abs(far->height_error) <= height_tolerance)
                    return PitchSolution{far_pitch, far->time};
                if (far && (far->height_error < ArithmeticType{0}) == (near->height_error < ArithmeticType{0}))
                    far.reset();
            }

            for (auto step = first_step; !far; step = std::min(step * ArithmeticType{2}, max_step))
            {
                if (direction > ArithmeticType{0} ? near_pitch >= pitch_limit : near_pitch <= lowest)
                    return std::nullopt;

                far_pitch = std::clamp(near_pitch + direction * step, lowest, pitch_limit);
                far = probe(far_pitch);

                // A lob this steep that still cannot cover the distance in time is over the top of what the weapon
                // can reach, or on the high arc past what the time limit allows
                if (!far)
                    return std::nullopt;
                if (std::abs(far->height_error) <= height_tolerance)
                    return PitchSolution{far_pitch, far->time};

                if ((far->height_error < ArithmeticType{0}) != (near->height_error < ArithmeticType{0}))
                    break;

                // Turning the pitch towards the peak has to bring the round up at the target. Once it stops doing
                // that the arc has gone over its peak without ever getting there: out of reach.
                if (direction == climb(arc) && far->height_error <= near->height_error)
                    return std::nullopt;

                near_pitch = far_pitch;
                near = far;
                far.reset();
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
        Arc m_arc;
    };
} // namespace omath::projectile_prediction
