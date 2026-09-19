// ProjectileFlight and ProjPredEngineDrag: rounds that lose speed to the air.
//
// The reference positions below were not computed, they were measured: Team Fortress 2's own vphysics.dll was run
// offline on the game's real grenade hull, launched with the calls the game makes, and sampled every tick. A pipe
// there leaves at forward * 1200 + up * 200 (1216.55 units/s, 9.46 degrees above the view) and the Loch-n-Load at
// forward * 1500 + up * 200, both under gravity 800 at a 0.015 s tick.
#include <cmath>
#include <gtest/gtest.h>
#include <omath/engines/unity_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/unreal_engine/traits/pred_engine_trait.hpp>
#include <omath/projectile_prediction/proj_pred_engine_drag.hpp>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>
#include <omath/projectile_prediction/projectile_flight.hpp>
#include <type_traits>

namespace
{
    using omath::Vector3;
    using omath::projectile_prediction::AimAngles;
    using omath::projectile_prediction::Launcher;
    using omath::projectile_prediction::Projectile;
    using omath::projectile_prediction::ProjectileFlight;
    using omath::projectile_prediction::ProjPredEngineDrag;
    using omath::projectile_prediction::ProjPredEngineInterface;
    using omath::projectile_prediction::ProjPredEngineLegacy;
    using omath::projectile_prediction::Target;
    using omath::source_engine::PredEngineTrait;

    constexpr float k_gravity = 800.f;
    constexpr float k_tick = 0.015f;

    // A pipe tumbles, so one number covers its drag. The Loch-n-Load's round is fired without spin and keeps the
    // attitude it left with, so its drag is the hull's own: end-on and side-on.
    constexpr Projectile<float> k_pipe{
            .m_launch_speed = 1216.5525f, .m_gravity_scale = 1.f, .m_drag = 2.838e-4f, .m_max_speed = 2000.f};
    constexpr Projectile<float> k_loch{.m_launch_speed = 1513.2745f,
                                       .m_gravity_scale = 1.f,
                                       .m_drag_forward = 9.911e-5f,
                                       .m_drag_up = 2.530e-4f,
                                       .m_max_speed = 2000.f};

    constexpr float k_pipe_pitch_offset = 9.46232f;
    constexpr float k_loch_pitch_offset = 7.59464f;

    // Muzzle at the eye, so a flight reads straight off against positions measured from the muzzle
    constexpr Launcher<float> k_pipe_at_eye{.eye_origin = {}, .launch_pitch_offset = k_pipe_pitch_offset};
    constexpr Launcher<float> k_loch_at_eye{.eye_origin = {}, .launch_pitch_offset = k_loch_pitch_offset};

    // The way the game really holds the weapon
    constexpr Launcher<float> k_pipe_launcher{.eye_origin = {0, 0, 64},
                                              .muzzle_offset = {.forward = 16, .right = 8, .up = -6},
                                              .launch_pitch_offset = k_pipe_pitch_offset};

    template<class Trait, class Type>
    [[nodiscard]]
    Vector3<Type> fly(const Projectile<Type>& round, const Launcher<Type>& launcher, const AimAngles<Type>& angles,
                      const Type gravity, const Type step, const int steps)
    {
        ProjectileFlight<Trait, Type> flight{round, launcher, angles, gravity, step};
        for (int i = 0; i < steps; ++i)
            flight.step();
        return flight.position();
    }

    // The same float expression evaluated in two places is not the same bits everywhere. GCC folds a constexpr
    // evaluation at compile time while the library runs it at run time, and a 32-bit x87 build carries extra precision
    // through whichever intermediates stayed in registers. So anything computed twice is compared within a tolerance;
    // plain == is kept for values that are copies of one another or exact in any precision.
    template<class Type>
    void expect_vectors_near(const Vector3<Type>& actual, const Vector3<Type>& expected, const double tolerance)
    {
        EXPECT_NEAR(actual.x, expected.x, tolerance);
        EXPECT_NEAR(actual.y, expected.y, tolerance);
        EXPECT_NEAR(actual.z, expected.z, tolerance);
    }

    // ---------------------------------------------------------------- Projectile

    TEST(ProjectileDrag, HasDragOnlyWhenACoefficientIsSet)
    {
        static_assert(!Projectile<float>{.m_launch_speed = 1000.f, .m_gravity_scale = 1.f}.has_drag());
        static_assert(!Projectile<float>{.m_max_speed = 2000.f}.has_drag());
        static_assert(Projectile<float>{.m_drag = 1e-4f}.has_drag());
        static_assert(Projectile<float>{.m_drag_forward = 1e-4f}.has_drag());
        static_assert(Projectile<double>{.m_drag_up = 1e-4}.has_drag());
        static_assert(k_pipe.has_drag() && k_loch.has_drag());
        static_assert(std::is_aggregate_v<Projectile<float>>);
        SUCCEED();
    }

    // ---------------------------------------------------------------- ProjectileFlight

    TEST(ProjectileFlight, StartsAtTheMuzzle)
    {
        constexpr AimAngles<float> angles{20.f, 35.f};
        const ProjectileFlight<> flight{k_pipe, k_pipe_launcher, angles, k_gravity, k_tick};

        const auto muzzle = k_pipe_launcher.launch_origin(PredEngineTrait::calc_view_basis(angles.pitch, angles.yaw));

        expect_vectors_near(flight.origin(), muzzle, 1e-3);
        EXPECT_EQ(flight.position(), flight.origin());
        EXPECT_FLOAT_EQ(flight.time(), 0.f);
    }

    TEST(ProjectileFlight, WithoutGravityOrDragItFliesStraight)
    {
        constexpr Projectile<float> round{.m_launch_speed = 1000.f};
        ProjectileFlight<> flight{round, Launcher<float>{}, {0.f, 0.f}, k_gravity, k_tick};

        for (int i = 0; i < 100; ++i)
            flight.step();

        EXPECT_NEAR(flight.position().x, 1500.f, 1e-2f);
        EXPECT_NEAR(flight.position().y, 0.f, 1e-3f);
        EXPECT_NEAR(flight.position().z, 0.f, 1e-3f);
        EXPECT_NEAR(flight.velocity().length(), 1000.f, 1e-3f);
        EXPECT_NEAR(flight.time(), 1.5f, 1e-6f);
    }

    TEST(ProjectileFlight, GravityLandsOnTheVelocityBeforeTheMove)
    {
        // The engine's step is semi-implicit: the velocity it moves with already carries this step's gravity. Over n
        // steps that falls g * dt^2 * n * (n + 1) / 2, a little further than the g * t^2 / 2 of a true parabola.
        constexpr Projectile<float> round{.m_launch_speed = 1000.f, .m_gravity_scale = 0.5f};
        constexpr int steps = 50;

        const auto position = fly<PredEngineTrait>(round, Launcher<float>{}, {0.f, 0.f}, k_gravity, k_tick, steps);

        constexpr float scaled_gravity = k_gravity * 0.5f;
        constexpr float fallen = scaled_gravity * k_tick * k_tick * steps * (steps + 1) / 2.f;

        EXPECT_NEAR(position.z, -fallen, 1e-2f);
        EXPECT_NEAR(position.x, 1000.f * k_tick * steps, 1e-2f);
    }

    TEST(ProjectileFlight, PitchOffsetRaisesTheRoundAndLeavesTheMuzzleAlone)
    {
        constexpr Projectile<float> round{.m_launch_speed = 1216.5525f};
        constexpr AimAngles<float> angles{10.f, 0.f};

        const ProjectileFlight<> flight{round, k_pipe_launcher, angles, k_gravity, k_tick};
        const auto& velocity = flight.velocity();

        // forward * 1200 + up * 200 in the view frame is one direction, 9.46 degrees above the view
        EXPECT_NEAR(velocity.length(), 1216.5525f, 1e-2f);
        EXPECT_NEAR(omath::angles::radians_to_degrees(std::atan2(velocity.z, velocity.x)),
                    angles.pitch + k_pipe_pitch_offset, 1e-3f);

        auto plain = k_pipe_launcher;
        plain.launch_pitch_offset = 0.f;
        expect_vectors_near(flight.origin(), ProjectileFlight<>{round, plain, angles, k_gravity, k_tick}.origin(),
                            1e-3);
    }

    TEST(ProjectileFlight, TumblingDragTakesItsShareOfTheSpeedEveryStep)
    {
        constexpr Projectile<float> round{.m_launch_speed = 1000.f, .m_drag = 2e-4f};
        ProjectileFlight<> flight{round, Launcher<float>{}, {0.f, 0.f}, k_gravity, k_tick};

        flight.step();
        const auto after_one = 1000.f * (1.f - 2e-4f * 1000.f * k_tick);
        EXPECT_NEAR(flight.velocity().length(), after_one, 1e-3f);

        // Proportional to the speed it has left, so every step takes a smaller bite than the one before
        flight.step();
        EXPECT_NEAR(flight.velocity().length(), after_one * (1.f - 2e-4f * after_one * k_tick), 1e-3f);
    }

    TEST(ProjectileFlight, AxisDragFollowsTheRoundsAttitude)
    {
        // Flying along its own nose the round only feels end-on drag...
        constexpr Projectile<float> round{.m_launch_speed = 1000.f, .m_drag_forward = 1e-4f, .m_drag_up = 4e-4f};
        ProjectileFlight<> nose_first{round, Launcher<float>{}, {30.f, 0.f}, k_gravity, k_tick};
        nose_first.step();
        EXPECT_NEAR(nose_first.velocity().length(), 1000.f * (1.f - 1e-4f * 1000.f * k_tick), 1e-3f);

        // ...and leaving 30 degrees above it, it feels cos(30) of that and sin(30) of the side-on drag
        ProjectileFlight<> raised{round, Launcher<float>{.launch_pitch_offset = 30.f}, {0.f, 0.f}, k_gravity, k_tick};
        raised.step();

        const auto rate = 1e-4f * 1000.f * std::cos(omath::angles::degrees_to_radians(30.f))
                          + 4e-4f * 1000.f * std::sin(omath::angles::degrees_to_radians(30.f));
        EXPECT_NEAR(raised.velocity().length(), 1000.f * (1.f - rate * k_tick), 1e-3f);
    }

    TEST(ProjectileFlight, DragCanStopTheRoundButNeverTurnItAround)
    {
        constexpr Projectile<float> round{.m_launch_speed = 1000.f, .m_drag = 10.f};
        ProjectileFlight<> flight{round, Launcher<float>{}, {0.f, 0.f}, k_gravity, k_tick};

        flight.step();

        EXPECT_FLOAT_EQ(flight.velocity().length(), 0.f);
        EXPECT_EQ(flight.position(), flight.origin());
    }

    TEST(ProjectileFlight, SpeedIsCappedOnLaunchAndAgainAfterEveryStep)
    {
        // A fully charged sticky asks for forward * 2400 + up * 200, and VPhysics lets it leave at 2000
        constexpr Projectile<float> charged{.m_launch_speed = 2408.319f, .m_gravity_scale = 1.f, .m_max_speed = 2000.f};
        ProjectileFlight<> flight{
                charged, Launcher<float>{.launch_pitch_offset = 4.76364f}, {-80.f, 0.f}, k_gravity, k_tick};

        EXPECT_NEAR(flight.velocity().length(), 2000.f, 1e-2f);

        // Fired almost straight down, gravity keeps trying to push it past the cap
        for (int i = 0; i < 20; ++i)
        {
            flight.step();
            EXPECT_LE(flight.velocity().length(), 2000.f + 1e-2f);
        }

        // Zero means there is no cap
        constexpr Projectile<float> uncapped{.m_launch_speed = 2408.319f};
        EXPECT_NEAR(
                (ProjectileFlight<>{uncapped, Launcher<float>{}, {0.f, 0.f}, k_gravity, k_tick}.velocity().length()),
                2408.319f, 1e-2f);
    }

    TEST(ProjectileFlight, FollowsSourcePhysicsForARoundThatHoldsItsAttitude)
    {
        // Nothing random about this one, so the engine's own numbers can be asked for almost exactly
        struct Sample
        {
            float view_pitch;
            int steps;
            float x;
            float z;
        };
        constexpr Sample measured[] = {{20.f, 40, 765.4679f, 257.2263f},   {20.f, 100, 1757.2307f, 96.5640f},
                                       {-10.f, 40, 860.7100f, -178.6139f}, {-10.f, 100, 1951.4204f, -891.4063f},
                                       {45.f, 40, 525.6462f, 544.0996f},   {45.f, 100, 1230.2048f, 767.2341f}};

        for (const auto& sample : measured)
        {
            const auto position = fly<PredEngineTrait>(k_loch, k_loch_at_eye, {sample.view_pitch, 0.f}, k_gravity,
                                                       k_tick, sample.steps);

            EXPECT_NEAR(position.x, sample.x, 0.1f) << "pitch " << sample.view_pitch << " step " << sample.steps;
            EXPECT_NEAR(position.z, sample.z, 0.1f) << "pitch " << sample.view_pitch << " step " << sample.steps;
            EXPECT_NEAR(position.y, 0.f, 1e-3f);
        }
    }

    TEST(ProjectileFlight, FollowsSourcePhysicsForATumblingRound)
    {
        // The game spins every pipe differently, which moves these by about one percent. The numbers are the middle
        // of that spread, and a parabola would be 145 and 430 units past them by the last two.
        struct Sample
        {
            float view_pitch;
            int steps;
            float x;
            float z;
        };
        constexpr Sample measured[] = {{0.f, 40, 653.008f, -29.273f},
                                       {0.f, 100, 1436.293f, -544.110f},
                                       {30.f, 40, 514.455f, 284.513f},
                                       {30.f, 100, 1161.768f, 147.345f}};

        for (const auto& sample : measured)
        {
            const auto position = fly<PredEngineTrait>(k_pipe, k_pipe_at_eye, {sample.view_pitch, 0.f}, k_gravity,
                                                       k_tick, sample.steps);

            EXPECT_NEAR(position.x, sample.x, 2.f) << "pitch " << sample.view_pitch << " step " << sample.steps;
            EXPECT_NEAR(position.z, sample.z, 2.f) << "pitch " << sample.view_pitch << " step " << sample.steps;
        }
    }

    TEST(ProjectileFlight, GravityPullsAlongWhicheverAxisTheEngineCallsUp)
    {
        // Unity keeps height on y and looks along z
        constexpr Projectile<float> round{.m_launch_speed = 40.f, .m_gravity_scale = 1.f, .m_drag = 0.01f};

        const auto position =
                fly<omath::unity_engine::PredEngineTrait>(round, Launcher<float>{}, {0.f, 0.f}, 9.81f, 0.02f, 50);

        EXPECT_LT(position.y, -4.f);
        EXPECT_NEAR(position.x, 0.f, 1e-3f);
        EXPECT_GT(position.z, 25.f);
        EXPECT_LT(position.z, 40.f); // drag kept it short of the 40 a vacuum would give
    }

    TEST(ProjectileFlight, WorksInDoublePrecision)
    {
        // Unreal's trait is a double one: x ahead, z up, centimetres
        constexpr Projectile<double> round{.m_launch_speed = 3000., .m_gravity_scale = 1., .m_drag = 1e-5};

        const auto position =
                fly<omath::unreal_engine::PredEngineTrait>(round, Launcher<double>{}, {0., 0.}, 980., 1. / 60., 60);

        static_assert(std::is_same_v<decltype(position.x), double>);
        EXPECT_GT(position.x, 2500.);
        EXPECT_LT(position.x, 3000.);
        EXPECT_LT(position.z, -450.);
    }

    // ---------------------------------------------------------------- ProjPredEngineDrag

    const ProjPredEngineDrag<> k_engine(k_gravity, k_tick, 2.13f, 5.f);

    TEST(ProjPredEngineDrag, LandsTheRoundOnAStationaryTarget)
    {
        constexpr Target<float> target{.m_origin = {900, 150, 40}, .m_velocity = {}, .m_is_airborne = false};

        const auto solution = k_engine.maybe_calculate_aim(k_pipe, k_pipe_launcher, target);
        ASSERT_TRUE(solution.has_value());

        const auto arrival = k_engine.predict_projectile_position(k_pipe, k_pipe_launcher, solution->angles,
                                                                  solution->time_of_flight);

        EXPECT_LT(arrival.distance_to(target.m_origin), 0.5f);
        EXPECT_EQ(solution->predicted_target_position, target.m_origin);
    }

    TEST(ProjPredEngineDrag, NeedsMoreLoftAndMoreTimeThanAParabola)
    {
        constexpr Target<float> target{.m_origin = {1000, 0, 64}, .m_velocity = {}, .m_is_airborne = false};

        auto vacuum_round = k_pipe;
        vacuum_round.m_drag = 0.f;

        const auto with_drag = k_engine.maybe_calculate_aim(k_pipe, k_pipe_launcher, target);
        const auto in_vacuum = ProjPredEngineLegacy<>(k_gravity, 0.001f, 2.13f, 5.f)
                                       .maybe_calculate_aim(vacuum_round, k_pipe_launcher, target);
        ASSERT_TRUE(with_drag.has_value());
        ASSERT_TRUE(in_vacuum.has_value());

        EXPECT_GT(with_drag->angles.pitch, in_vacuum->angles.pitch + 3.f);
        EXPECT_GT(with_drag->time_of_flight, in_vacuum->time_of_flight + 0.1f);
    }

    TEST(ProjPredEngineDrag, WithoutDragItAgreesWithTheClosedForm)
    {
        constexpr Target<float> target{.m_origin = {600, -200, 100}, .m_velocity = {}, .m_is_airborne = false};

        auto vacuum_round = k_pipe;
        vacuum_round.m_drag = 0.f;

        const auto stepped = k_engine.maybe_calculate_aim(vacuum_round, k_pipe_launcher, target);
        const auto closed_form = ProjPredEngineLegacy<>(k_gravity, 0.001f, 2.13f, 5.f)
                                         .maybe_calculate_aim(vacuum_round, k_pipe_launcher, target);
        ASSERT_TRUE(stepped.has_value());
        ASSERT_TRUE(closed_form.has_value());

        // What is left between them is the extra the stepped fall drops: see GravityLandsOnTheVelocityBeforeTheMove
        EXPECT_NEAR(stepped->angles.pitch, closed_form->angles.pitch, 0.5f);
        EXPECT_NEAR(stepped->angles.yaw, closed_form->angles.yaw, 0.05f);
        EXPECT_NEAR(stepped->time_of_flight, closed_form->time_of_flight, 0.01f);
    }

    TEST(ProjPredEngineDrag, LeadsAMovingTarget)
    {
        constexpr Target<float> target{.m_origin = {800, 0, 64}, .m_velocity = {-120, 300, 0}, .m_is_airborne = false};

        const auto solution = k_engine.maybe_calculate_aim(k_pipe, k_pipe_launcher, target);
        ASSERT_TRUE(solution.has_value());

        // It aims at where the target will be when the round gets there, and the round does get there
        const auto expected = PredEngineTrait::predict_target_position(target, solution->time_of_flight, k_gravity);
        expect_vectors_near(solution->predicted_target_position, expected, 1e-3);
        EXPECT_GT(solution->predicted_target_position.y, 150.f);

        const auto arrival = k_engine.predict_projectile_position(k_pipe, k_pipe_launcher, solution->angles,
                                                                  solution->time_of_flight);
        EXPECT_LT(arrival.distance_to(expected), 0.5f);
    }

    TEST(ProjPredEngineDrag, LeadsAnAirborneTargetDownItsFall)
    {
        constexpr Target<float> target{
                .m_origin = {700, 100, 300}, .m_velocity = {50, -80, 200}, .m_is_airborne = true};

        const auto solution = k_engine.maybe_calculate_aim(k_loch, k_pipe_launcher, target);
        ASSERT_TRUE(solution.has_value());

        const auto expected = PredEngineTrait::predict_target_position(target, solution->time_of_flight, k_gravity);
        const auto arrival = k_engine.predict_projectile_position(k_loch, k_pipe_launcher, solution->angles,
                                                                  solution->time_of_flight);

        EXPECT_LT(arrival.distance_to(expected), 0.5f);
    }

    TEST(ProjPredEngineDrag, LandsATargetFlungAtTheShooterFasterThanTheRoundFlies)
    {
        // Blown towards the shooter at 1400 units/s, against a pipe that covers ground at about 1000. Feeding each
        // arrival time back in as the next guess overshoots by more every pass here, so the time has to be solved for
        // rather than iterated on.
        constexpr Target<float> target{
                .m_origin = {1068, 0, 250}, .m_velocity = {-1422, -224, 354}, .m_is_airborne = true};

        const auto solution = k_engine.maybe_calculate_aim(k_pipe, k_pipe_launcher, target);
        ASSERT_TRUE(solution.has_value());

        const auto expected = PredEngineTrait::predict_target_position(target, solution->time_of_flight, k_gravity);
        const auto arrival = k_engine.predict_projectile_position(k_pipe, k_pipe_launcher, solution->angles,
                                                                  solution->time_of_flight);

        EXPECT_LT(arrival.distance_to(expected), 0.5f);
    }

    TEST(ProjPredEngineDrag, CatchesATargetFleeingNearlyAsFastAsTheRound)
    {
        // A rocket has neither drag nor drop, and flying one a step at a time is exact for it. The target is going
        // away at two thirds of its speed, which is a shot that can be made but takes a long time to close on.
        constexpr Projectile<float> rocket{.m_launch_speed = 1980.f};
        constexpr Launcher<float> launcher{.eye_origin = {0, 0, 64},
                                           .muzzle_offset = {.forward = 23.5f, .right = 12, .up = -3}};
        constexpr Target<float> target{
                .m_origin = {1280, 0, 250}, .m_velocity = {1263, -283, 528}, .m_is_airborne = true};

        const ProjPredEngineDrag<> engine(k_gravity, k_tick, 3.f, 5.f);

        const auto solution = engine.maybe_calculate_aim(rocket, launcher, target);
        ASSERT_TRUE(solution.has_value());

        const auto expected = PredEngineTrait::predict_target_position(target, solution->time_of_flight, k_gravity);
        const auto arrival =
                engine.predict_projectile_position(rocket, launcher, solution->angles, solution->time_of_flight);

        EXPECT_LT(arrival.distance_to(expected), 0.5f);
        EXPECT_GT(solution->time_of_flight, 1.5f);
    }

    TEST(ProjPredEngineDrag, ReachIsTheOneTheGameGivesThePipe)
    {
        // Measured in the game's physics, a pipe fired at a level target gets 1352 units and no further. A parabola
        // says 1850, which is 500 units of shots that cannot be made.
        constexpr Launcher<float> launcher{.eye_origin = {},
                                           .muzzle_offset = {.forward = 16, .right = 8, .up = -6},
                                           .launch_pitch_offset = k_pipe_pitch_offset};

        const auto at = [&](const float distance)
        {
            return k_engine.maybe_calculate_aim(k_pipe, launcher, Target<float>{.m_origin = {distance, 0, 0}});
        };

        EXPECT_TRUE(at(1340.f).has_value());
        EXPECT_FALSE(at(1365.f).has_value());
        EXPECT_FALSE(at(1800.f).has_value());
        EXPECT_FALSE(at(5000.f).has_value());
    }

    TEST(ProjPredEngineDrag, KeepsToTheHorizon)
    {
        constexpr Target<float> target{.m_origin = {1200, 0, 64}, .m_velocity = {}, .m_is_airborne = false};

        const auto unhurried = k_engine.maybe_calculate_aim(k_pipe, k_pipe_launcher, target);
        ASSERT_TRUE(unhurried.has_value());
        ASSERT_GT(unhurried->time_of_flight, 1.2f);

        // The same shot, asked of an engine whose rounds are gone by then
        EXPECT_FALSE(ProjPredEngineDrag<>(k_gravity, k_tick, 1.2f, 5.f)
                             .maybe_calculate_aim(k_pipe, k_pipe_launcher, target)
                             .has_value());
    }

    TEST(ProjPredEngineDrag, ReportsViewAnglesWithTheAimPointOnTheirRay)
    {
        constexpr Target<float> target{.m_origin = {600, 300, 200}, .m_velocity = {}, .m_is_airborne = false};

        const auto solution = k_engine.maybe_calculate_aim(k_pipe, k_pipe_launcher, target);
        ASSERT_TRUE(solution.has_value());

        const auto& eye = k_pipe_launcher.eye_origin;
        EXPECT_NEAR(PredEngineTrait::calc_direct_pitch_angle(eye, solution->aim_point), solution->angles.pitch, 1e-2f);
        EXPECT_NEAR(PredEngineTrait::calc_direct_yaw_angle(eye, solution->aim_point), solution->angles.yaw, 1e-2f);
        EXPECT_NEAR(eye.distance_to(solution->aim_point), eye.distance_to(target.m_origin), 1e-2f);

        // The offset comes off the view, not off the round: the same shot from a weapon that fires along its
        // crosshair needs the view that much higher. Not to the last hundredth, because the muzzle hangs off the
        // view and so sits in a slightly different place for the two.
        auto along_the_crosshair = k_pipe_launcher;
        along_the_crosshair.launch_pitch_offset = 0.f;

        const auto plain = k_engine.maybe_calculate_aim(k_pipe, along_the_crosshair, target);
        ASSERT_TRUE(plain.has_value());
        EXPECT_NEAR(plain->angles.pitch, solution->angles.pitch + k_pipe_pitch_offset, 0.3f);
        EXPECT_NEAR(plain->time_of_flight, solution->time_of_flight, 0.01f);
    }

    TEST(ProjPredEngineDrag, RefusesAViewTheEngineWillNotAllow)
    {
        // Almost straight overhead: the pitch that would do it is past what Source lets a player look at
        constexpr Target<float> target{.m_origin = {6, 0, 464}, .m_velocity = {}, .m_is_airborne = false};

        EXPECT_FALSE(
                k_engine.maybe_calculate_aim(k_pipe, Launcher<float>{.eye_origin = {0, 0, 64}}, target).has_value());
    }

    TEST(ProjPredEngineDrag, SettingsThatCannotRunGiveNoSolution)
    {
        constexpr Target<float> target{.m_origin = {500, 0, 64}, .m_velocity = {}, .m_is_airborne = false};

        EXPECT_FALSE(ProjPredEngineDrag<>(k_gravity, 0.f, 2.f, 5.f)
                             .maybe_calculate_aim(k_pipe, k_pipe_launcher, target)
                             .has_value());
        EXPECT_FALSE(ProjPredEngineDrag<>(k_gravity, -k_tick, 2.f, 5.f)
                             .maybe_calculate_aim(k_pipe, k_pipe_launcher, target)
                             .has_value());
        EXPECT_FALSE(ProjPredEngineDrag<>(k_gravity, k_tick, 0.f, 5.f)
                             .maybe_calculate_aim(k_pipe, k_pipe_launcher, target)
                             .has_value());

        constexpr Projectile<float> dud{.m_gravity_scale = 1.f, .m_drag = 2.838e-4f};
        EXPECT_FALSE(k_engine.maybe_calculate_aim(dud, k_pipe_launcher, target).has_value());
    }

    TEST(ProjPredEngineDrag, IsAnOrdinaryValueBehindTheInterface)
    {
        static_assert(std::is_copy_constructible_v<ProjPredEngineDrag<>>);
        static_assert(std::is_copy_assignable_v<ProjPredEngineDrag<>>);
        static_assert(std::is_nothrow_move_constructible_v<ProjPredEngineDrag<>>);

        constexpr Target<float> target{.m_origin = {700, 120, 90}, .m_velocity = {-60, 45, 0}, .m_is_airborne = false};
        static_assert(noexcept(k_engine.maybe_calculate_aim(k_pipe, k_pipe_launcher, target)));

        const ProjPredEngineInterface<float>& through_base = k_engine;
        const auto copy = k_engine; // NOLINT(*-unnecessary-copy-initialization)

        const auto direct = k_engine.maybe_calculate_aim(k_pipe, k_pipe_launcher, target);
        const auto dispatched = through_base.maybe_calculate_aim(k_pipe, k_pipe_launcher, target);
        const auto from_copy = copy.maybe_calculate_aim(k_pipe, k_pipe_launcher, target);
        ASSERT_TRUE(direct.has_value());
        ASSERT_TRUE(dispatched.has_value());
        ASSERT_TRUE(from_copy.has_value());

        // The solver iterates to a tolerance, so two differently compiled runs of it may stop a refinement apart. A
        // copy or a dispatch that lost the engine's settings would be out by degrees, not by this.
        EXPECT_NEAR(direct->angles.pitch, dispatched->angles.pitch, 0.02f);
        EXPECT_NEAR(direct->angles.yaw, dispatched->angles.yaw, 0.02f);
        EXPECT_NEAR(direct->time_of_flight, from_copy->time_of_flight, 5e-3f);
        expect_vectors_near(direct->aim_point, from_copy->aim_point, 0.5);
    }

    TEST(ProjPredEngineDrag, SolvesInAnEngineWithHeightOnY)
    {
        using Trait = omath::unity_engine::PredEngineTrait;

        // Metres, Unity's 50 Hz physics, and a round that sheds about a third of its speed on the way
        constexpr Projectile<float> round{.m_launch_speed = 40.f, .m_gravity_scale = 1.f, .m_drag = 0.01f};
        constexpr Launcher<float> launcher{.eye_origin = {0, 1.7f, 0},
                                           .muzzle_offset = {.forward = 0.4f, .right = 0.15f, .up = -0.1f}};
        constexpr Target<float> target{.m_origin = {8, 1, 30}, .m_velocity = {-3, 0, 1}, .m_is_airborne = false};

        const ProjPredEngineDrag<Trait> engine(9.81f, 0.02f, 5.f, 0.05f);

        const auto solution = engine.maybe_calculate_aim(round, launcher, target);
        ASSERT_TRUE(solution.has_value());

        const auto expected = Trait::predict_target_position(target, solution->time_of_flight, 9.81f);
        const auto arrival =
                engine.predict_projectile_position(round, launcher, solution->angles, solution->time_of_flight);

        EXPECT_LT(arrival.distance_to(expected), 0.01f);
        EXPECT_GT(solution->angles.pitch, Trait::calc_direct_pitch_angle(launcher.eye_origin, expected));
    }

    TEST(ProjPredEngineDrag, SolvesInDoublePrecision)
    {
        using Trait = omath::unreal_engine::PredEngineTrait;

        constexpr Projectile<double> round{.m_launch_speed = 3000., .m_gravity_scale = 1., .m_drag = 1e-5};
        constexpr Launcher<double> launcher{.eye_origin = {0, 0, 170}, .muzzle_offset = {.forward = 40, .right = 15}};
        constexpr Target<double> target{
                .m_origin = {4000, 500, 300}, .m_velocity = {0, -300, 0}, .m_is_airborne = false};

        const ProjPredEngineDrag<Trait, double> engine(980., 1. / 60., 5., 5.);

        const auto solution = engine.maybe_calculate_aim(round, launcher, target);
        ASSERT_TRUE(solution.has_value());

        const auto expected = Trait::predict_target_position(target, solution->time_of_flight, 980.);
        const auto arrival =
                engine.predict_projectile_position(round, launcher, solution->angles, solution->time_of_flight);

        EXPECT_LT(arrival.distance_to(expected), 0.5);
    }
} // namespace
