// Properties of ProjPredEngineLegacy's answers that hold whatever the numbers are: symmetries of the scene, how the
// answer moves with range and speed, what the horizon and a rejected step do, and how the answer lines up with a
// camera. Source trait throughout; the per-engine checks live in tests/engines/unit_test_pred_engine_traits_all.cpp.
#include <cmath>
#include <gtest/gtest.h>
#include <initializer_list>
#include <omath/engines/source_engine/camera.hpp>
#include <omath/engines/source_engine/traits/camera_trait.hpp>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>

namespace
{
    using omath::Vector3;
    using Projectile = omath::projectile_prediction::Projectile<float>;
    using Target = omath::projectile_prediction::Target<float>;
    using Launcher = omath::projectile_prediction::Launcher<float>;
    using MuzzleOffset = omath::projectile_prediction::MuzzleOffset<float>;
    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<>;
    using Trait = omath::source_engine::PredEngineTrait;

    constexpr float k_gravity = 800.f;
    constexpr float k_step = 1.f / 1000.f;
    constexpr MuzzleOffset k_muzzle{.forward = 16.f, .right = 8.f, .up = -6.f};

    Engine make_engine(const float horizon = 10.f, const float tolerance = 5.f, const float step = k_step)
    {
        return Engine(k_gravity, step, horizon, tolerance);
    }

    float pipe_pitch_offset()
    {
        return omath::angles::radians_to_degrees(std::atan2(200.f, 1200.f));
    }

    float angle_difference(const float a, const float b)
    {
        return std::remainder(a - b, 360.f);
    }

    // Source is Z up, so this turns the scene about the vertical through the origin
    Vector3<float> rotate_about_up(const Vector3<float>& v, const float degrees)
    {
        const float radians = omath::angles::degrees_to_radians(degrees);
        return {v.x * std::cos(radians) - v.y * std::sin(radians), v.x * std::sin(radians) + v.y * std::cos(radians),
                v.z};
    }

    // ---------------------------------------------------------------- symmetries

    TEST(PredictionProperties, TurningTheSceneAboutTheEyeOnlyShiftsTheYaw)
    {
        constexpr Projectile round{.m_launch_speed = 1500.f, .m_gravity_scale = 1.f};
        // The muzzle offset turns with the view, so the scene stays symmetric with it in place
        constexpr Launcher launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = k_muzzle};
        constexpr Target target{.m_origin = {700, 120, 90}, .m_velocity = {-60, 45, 0}, .m_is_airborne = false};

        const auto engine = make_engine();
        const auto reference = engine.maybe_calculate_aim(round, launcher, target);
        ASSERT_TRUE(reference.has_value());

        for (const float turn : {37.f, 143.f, -90.f, 180.f})
        {
            const Target turned{.m_origin = rotate_about_up(target.m_origin, turn),
                                .m_velocity = rotate_about_up(target.m_velocity, turn),
                                .m_is_airborne = false};

            const auto aim = engine.maybe_calculate_aim(round, launcher, turned);
            ASSERT_TRUE(aim.has_value()) << "turned by " << turn;

            EXPECT_NEAR(aim->angles.pitch, reference->angles.pitch, 0.02f) << "turned by " << turn;
            EXPECT_NEAR(angle_difference(aim->angles.yaw, reference->angles.yaw + turn), 0.f, 0.02f)
                    << "turned by " << turn;
            EXPECT_NEAR(aim->time_of_flight, reference->time_of_flight, 1.5f * k_step) << "turned by " << turn;
        }
    }

    TEST(PredictionProperties, AWorldOffsetDoesNotTurnWithTheScene)
    {
        // Same turn, but the spawn offset is fixed in the world: 40 units to the side when facing +X, 40 units ahead
        // once the scene has turned a quarter. The bearing to the target changes by about atan(40 / 700) on top of the
        // turn, which is the whole difference between world_offset and muzzle_offset.
        constexpr Projectile round{.m_launch_speed = 1500.f, .m_gravity_scale = 1.f};
        constexpr Launcher launcher{.eye_origin = {0, 0, 64}, .world_offset = {0, 40, 0}};
        constexpr Target target{.m_origin = {700, 0, 64}, .m_velocity = {}, .m_is_airborne = false};

        const auto engine = make_engine();
        const auto reference = engine.maybe_calculate_aim(round, launcher, target);
        const auto turned = engine.maybe_calculate_aim(
                round, launcher,
                Target{.m_origin = rotate_about_up(target.m_origin, 90.f), .m_velocity = {}, .m_is_airborne = false});
        ASSERT_TRUE(reference.has_value());
        ASSERT_TRUE(turned.has_value());

        EXPECT_GT(std::abs(angle_difference(turned->angles.yaw, reference->angles.yaw + 90.f)), 2.f);
    }

    TEST(PredictionProperties, MovingTheWholeSceneMovesOnlyThePoints)
    {
        constexpr Projectile round{.m_launch_speed = 1500.f, .m_gravity_scale = 1.f};
        constexpr Target target{.m_origin = {700, 120, 90}, .m_velocity = {-60, 45, 0}, .m_is_airborne = false};
        constexpr Vector3<float> eye{0, 0, 64};
        constexpr Vector3<float> shift{1000, -2000, 300};

        const auto engine = make_engine();
        const auto reference =
                engine.maybe_calculate_aim(round, Launcher{.eye_origin = eye, .muzzle_offset = k_muzzle}, target);
        const auto moved = engine.maybe_calculate_aim(
                round, Launcher{.eye_origin = eye + shift, .muzzle_offset = k_muzzle},
                Target{.m_origin = target.m_origin + shift, .m_velocity = target.m_velocity, .m_is_airborne = false});
        ASSERT_TRUE(reference.has_value());
        ASSERT_TRUE(moved.has_value());

        EXPECT_NEAR(moved->angles.pitch, reference->angles.pitch, 0.02f);
        EXPECT_NEAR(moved->angles.yaw, reference->angles.yaw, 0.02f);
        EXPECT_NEAR(moved->time_of_flight, reference->time_of_flight, 1.5f * k_step);
        EXPECT_LE((moved->aim_point - shift).distance_to(reference->aim_point), 1.f);
        EXPECT_LE((moved->predicted_target_position - shift).distance_to(reference->predicted_target_position), 0.2f);
    }

    TEST(PredictionProperties, MirroringTheSceneFlipsTheYaw)
    {
        constexpr Projectile round{.m_launch_speed = 1500.f, .m_gravity_scale = 1.f};
        // Nothing off to one side on the shooter, so left and right are the same problem
        constexpr Launcher launcher{.eye_origin = {0, 0, 64},
                                    .muzzle_offset = {.forward = 16.f, .right = 0.f, .up = -6.f}};
        constexpr Target target{.m_origin = {700, 120, 90}, .m_velocity = {-60, 45, -5}, .m_is_airborne = false};
        constexpr Target mirrored{.m_origin = {700, -120, 90}, .m_velocity = {-60, -45, -5}, .m_is_airborne = false};

        const auto engine = make_engine();
        const auto aim = engine.maybe_calculate_aim(round, launcher, target);
        const auto mirror = engine.maybe_calculate_aim(round, launcher, mirrored);
        ASSERT_TRUE(aim.has_value());
        ASSERT_TRUE(mirror.has_value());

        EXPECT_NEAR(mirror->angles.pitch, aim->angles.pitch, 1e-3f);
        EXPECT_NEAR(mirror->angles.yaw, -aim->angles.yaw, 1e-3f);
        EXPECT_FLOAT_EQ(mirror->time_of_flight, aim->time_of_flight);
    }

    // ---------------------------------------------------------------- shape of the answer

    TEST(PredictionProperties, TimeOfFlightIsAWholeNumberOfStepsAndTheAimPointIsAsFarAsTheTarget)
    {
        constexpr Projectile round{.m_launch_speed = 1100.f, .m_gravity_scale = 0.4f};
        constexpr Launcher launcher{.eye_origin = {10, -20, 64}, .muzzle_offset = k_muzzle};
        constexpr Target target{.m_origin = {900, 250, 0}, .m_velocity = {-120, 80, 0}, .m_is_airborne = false};

        for (const float step : {1.f / 1000.f, 1.f / 250.f, 1.f / 64.f})
        {
            const auto aim = make_engine(10.f, 20.f, step).maybe_calculate_aim(round, launcher, target);
            ASSERT_TRUE(aim.has_value()) << "step " << step;

            const float steps = aim->time_of_flight / step;
            EXPECT_NEAR(steps, std::round(steps), 1e-2f) << "step " << step;

            EXPECT_NEAR(launcher.eye_origin.distance_to(aim->aim_point),
                        launcher.eye_origin.distance_to(aim->predicted_target_position), 0.05f);

            const auto expected_target = target.m_origin + target.m_velocity * aim->time_of_flight;
            EXPECT_LE(aim->predicted_target_position.distance_to(expected_target), 1e-2f);
        }
    }

    TEST(PredictionProperties, FartherTargetsTakeLongerAndNeedMoreArc)
    {
        constexpr Projectile round{.m_launch_speed = 1500.f, .m_gravity_scale = 1.f};
        constexpr Launcher launcher{.eye_origin = {0, 0, 64}};
        const auto engine = make_engine();

        float previous_time = 0.f;
        float previous_pitch = -90.f;
        for (const float distance : {400.f, 800.f, 1200.f, 1600.f, 2000.f})
        {
            const auto aim = engine.maybe_calculate_aim(
                    round, launcher, Target{.m_origin = {distance, 0, 64}, .m_velocity = {}, .m_is_airborne = false});
            ASSERT_TRUE(aim.has_value()) << "distance " << distance;

            EXPECT_GT(aim->time_of_flight, previous_time) << "distance " << distance;
            EXPECT_GT(aim->angles.pitch, previous_pitch) << "distance " << distance;
            EXPECT_NEAR(aim->angles.yaw, 0.f, 1e-3f);

            previous_time = aim->time_of_flight;
            previous_pitch = aim->angles.pitch;
        }
    }

    TEST(PredictionProperties, AFasterRoundNeedsLessLeadAndLessArc)
    {
        constexpr Launcher launcher{.eye_origin = {0, 0, 64}};
        constexpr Target crossing{.m_origin = {800, 0, 64}, .m_velocity = {0, 200, 0}, .m_is_airborne = false};
        const auto engine = make_engine();

        const auto slow = engine.maybe_calculate_aim(Projectile{.m_launch_speed = 1000.f, .m_gravity_scale = 1.f},
                                                     launcher, crossing);
        const auto fast = engine.maybe_calculate_aim(Projectile{.m_launch_speed = 3000.f, .m_gravity_scale = 1.f},
                                                     launcher, crossing);
        ASSERT_TRUE(slow.has_value());
        ASSERT_TRUE(fast.has_value());

        EXPECT_GT(slow->angles.yaw, fast->angles.yaw); // target runs towards +Y, which is a positive yaw in Source
        EXPECT_GT(fast->angles.yaw, 0.f);
        EXPECT_GT(slow->angles.pitch, fast->angles.pitch);
        EXPECT_GT(slow->time_of_flight, fast->time_of_flight);
    }

    TEST(PredictionProperties, ATighterToleranceNeverGivesALooserHit)
    {
        constexpr Projectile round{.m_launch_speed = 2000.f, .m_gravity_scale = 0.5f};
        constexpr Launcher launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = k_muzzle};
        constexpr Target target{.m_origin = {1200, -300, 150}, .m_velocity = {30, 60, 0}, .m_is_airborne = false};

        for (const float tolerance : {40.f, 10.f, 2.f})
        {
            const auto aim = make_engine(10.f, tolerance).maybe_calculate_aim(round, launcher, target);
            ASSERT_TRUE(aim.has_value()) << "tolerance " << tolerance;

            const auto basis = Trait::calc_view_basis(aim->angles.pitch, aim->angles.yaw);
            const auto position =
                    Trait::predict_projectile_position(launcher.launch_origin(basis), round, aim->angles.pitch,
                                                       aim->angles.yaw, aim->time_of_flight, k_gravity);
            // +1: the muzzle is placed from the previous pass's angles, which costs a hair
            EXPECT_LE(position.distance_to(aim->predicted_target_position), tolerance + 1.f)
                    << "tolerance " << tolerance;
        }
    }

    // ---------------------------------------------------------------- horizon and rejected steps

    TEST(PredictionProperties, AHorizonShorterThanTheFlightFindsNothing)
    {
        constexpr Projectile round{.m_launch_speed = 1500.f, .m_gravity_scale = 0.5f};
        constexpr Launcher launcher{.eye_origin = {0, 0, 64}};
        constexpr Target target{.m_origin = {3000, 0, 64}, .m_velocity = {}, .m_is_airborne = false};

        EXPECT_FALSE(make_engine(1.5f).maybe_calculate_aim(round, launcher, target).has_value());

        const auto aim = make_engine(3.f).maybe_calculate_aim(round, launcher, target);
        ASSERT_TRUE(aim.has_value());
        EXPECT_GT(aim->time_of_flight, 1.9f);
        EXPECT_LT(aim->time_of_flight, 2.3f);
    }

    TEST(PredictionProperties, ARejectedStepDoesNotEndTheScan)
    {
        // A tolerance this wide lets every step through the reach check, so the only thing deciding the answer is
        // whether the view pitch can be set. The target starts almost straight below (view about -97.7 with the
        // offset, past Source's -89) and walks out from under the shooter; the first step it can be aimed at wins.
        constexpr float step = 0.05f;
        constexpr Projectile round{.m_launch_speed = 1100.f, .m_gravity_scale = 0.f};
        constexpr Target target{.m_origin = {30, 0, 0}, .m_velocity = {300, 0, 0}, .m_is_airborne = false};
        constexpr Vector3<float> eye{0, 0, 1000};
        const float offset = pipe_pitch_offset();

        const Engine engine(k_gravity, step, 5.f, 1e9f);

        const auto plain = engine.maybe_calculate_aim(round, Launcher{.eye_origin = eye}, target);
        ASSERT_TRUE(plain.has_value());
        EXPECT_FLOAT_EQ(plain->time_of_flight, 0.f);

        const auto aim =
                engine.maybe_calculate_aim(round, Launcher{.eye_origin = eye, .launch_pitch_offset = offset}, target);
        ASSERT_TRUE(aim.has_value());
        EXPECT_GT(aim->time_of_flight, 0.f);
        EXPECT_GE(aim->angles.pitch, -89.1f);

        // ...and it is the first such step: one step earlier the view would still have been out of reach
        const auto one_step_earlier = Trait::predict_target_position(target, aim->time_of_flight - step, k_gravity);
        EXPECT_LT(Trait::calc_direct_pitch_angle(eye, one_step_earlier) - offset, -89.1f);
    }

    TEST(PredictionProperties, AnOffsetDecidesWhetherAnOverheadShotCanBeSetUp)
    {
        // Straight up needs a launch pitch of 90. A weapon that throws 9.46 degrees above the crosshair gets there
        // from a view of 80.54; one that throws below it would need a view of 99.46.
        constexpr Projectile round{.m_launch_speed = 500.f, .m_gravity_scale = 1.f};
        constexpr Target overhead{.m_origin = {0, 0, 100}, .m_velocity = {}, .m_is_airborne = false};
        const Engine engine(400.f, k_step, 5.f, 5.f);
        const float offset = pipe_pitch_offset();

        const auto upward =
                engine.maybe_calculate_aim(round, Launcher{.eye_origin = {}, .launch_pitch_offset = offset}, overhead);
        ASSERT_TRUE(upward.has_value());
        EXPECT_NEAR(upward->angles.pitch, 90.f - offset, 0.01f);

        EXPECT_FALSE(
                engine.maybe_calculate_aim(round, Launcher{.eye_origin = {}, .launch_pitch_offset = -offset}, overhead)
                        .has_value());
    }

    TEST(PredictionProperties, WithoutGravityTheViewIsTheDirectAngleMinusTheOffset)
    {
        constexpr Projectile rocket{.m_launch_speed = 1100.f, .m_gravity_scale = 0.f};
        constexpr Vector3<float> eye{0, 0, 64};
        constexpr Target target{.m_origin = {600, -250, 300}, .m_velocity = {}, .m_is_airborne = false};

        for (const float offset : {0.f, 5.f, -5.f, pipe_pitch_offset()})
        {
            const auto aim = make_engine().maybe_calculate_aim(
                    rocket, Launcher{.eye_origin = eye, .launch_pitch_offset = offset}, target);
            ASSERT_TRUE(aim.has_value()) << "offset " << offset;

            EXPECT_NEAR(aim->angles.pitch, Trait::calc_direct_pitch_angle(eye, target.m_origin) - offset, 1e-4f);
            EXPECT_NEAR(aim->angles.yaw, Trait::calc_direct_yaw_angle(eye, target.m_origin), 1e-4f);
        }
    }

    // ---------------------------------------------------------------- offsets and the old call style

    TEST(PredictionProperties, AWorldOffsetBelowTheEyeAimsHigher)
    {
        constexpr Projectile round{.m_launch_speed = 1500.f, .m_gravity_scale = 0.5f};
        constexpr Target target{.m_origin = {900, 100, 64}, .m_velocity = {}, .m_is_airborne = false};
        const auto engine = make_engine();

        const auto from_the_eye = engine.maybe_calculate_aim(round, Launcher{.eye_origin = {0, 0, 64}}, target);
        const auto from_below = engine.maybe_calculate_aim(
                round, Launcher{.eye_origin = {0, 0, 64}, .world_offset = {0, 0, -40}}, target);
        ASSERT_TRUE(from_the_eye.has_value());
        ASSERT_TRUE(from_below.has_value());

        // 40 units lower over 900 is about 2.5 degrees more elevation
        EXPECT_NEAR(from_below->angles.pitch - from_the_eye->angles.pitch,
                    omath::angles::radians_to_degrees(std::atan2(40.f, 905.f)), 0.1f);
        EXPECT_NEAR(from_below->angles.yaw, from_the_eye->angles.yaw, 1e-3f);

        // The aim point still belongs to the eye, not to the muzzle
        const auto [pitch, yaw, roll] =
                omath::source_engine::CameraTrait::calc_look_at_angle({0, 0, 64}, from_below->aim_point);
        EXPECT_NEAR(-pitch.as_degrees(), from_below->angles.pitch, 0.01f);
        EXPECT_NEAR(yaw.as_degrees(), from_below->angles.yaw, 0.01f);
    }

    TEST(PredictionProperties, OldStyleAimPointStaysOnTheRayTheOldFormulaGave)
    {
        // Before Launcher the aim point was (target.x, target.y, eye.z + ground_distance * tan(pitch)). The wrapper now
        // returns a point at the target's distance along the view ray instead. Code that only looks at it (sets angles
        // from it, projects it) must see no difference, so the two points have to lie on one ray from the eye.
        constexpr Projectile round{
                .m_origin = {3, 2, 1}, .m_launch_offset = {}, .m_launch_speed = 3000.f, .m_gravity_scale = 0.6f};
        constexpr Target target{.m_origin = {900, -350, 240}, .m_velocity = {-35, 50, 0}, .m_is_airborne = false};
        const auto engine = make_engine();

        const auto point = engine.maybe_calculate_aim_point(round, target);
        const auto angles = engine.maybe_calculate_aim_angles(round, target);
        const auto solution = engine.maybe_calculate_aim(round, Engine::launcher_from_projectile(round), target);
        ASSERT_TRUE(point.has_value());
        ASSERT_TRUE(angles.has_value());
        ASSERT_TRUE(solution.has_value());

        const auto& predicted = solution->predicted_target_position;
        const float ground_distance = Trait::calc_vector_2d_distance(predicted - round.m_origin);
        const Vector3<float> old_point{
                predicted.x, predicted.y,
                round.m_origin.z + ground_distance * std::tan(omath::angles::degrees_to_radians(angles->pitch))};

        const auto new_direction = (point.value() - round.m_origin).normalized();
        const auto old_direction = (old_point - round.m_origin).normalized();
        EXPECT_LE(new_direction.distance_to(old_direction), 1e-4f);
    }

    // ---------------------------------------------------------------- together with a camera

    TEST(PredictionProperties, TheSolvedViewPutsTheAimPointUnderTheCrosshair)
    {
        using namespace omath::source_engine;

        const Projectile pipe{.m_launch_speed = std::hypot(1200.f, 200.f), .m_gravity_scale = 0.5f};
        const Launcher launcher{
                .eye_origin = {0, 0, 64}, .muzzle_offset = k_muzzle, .launch_pitch_offset = pipe_pitch_offset()};
        constexpr Target target{.m_origin = {600, 150, 0}, .m_velocity = {-40, 30, 0}, .m_is_airborne = false};

        const auto aim = make_engine().maybe_calculate_aim(pipe, launcher, target);
        ASSERT_TRUE(aim.has_value());

        // Source's own pitch is negative upwards
        const Camera camera(launcher.eye_origin,
                            ViewAngles{PitchAngle::from_degrees(-aim->angles.pitch),
                                       YawAngle::from_degrees(aim->angles.yaw), RollAngle::from_degrees(0.f)},
                            {1920.f, 1080.f}, omath::projection::FieldOfView::from_degrees(90.f), 0.01f, 10000.f);

        const auto crosshair = camera.world_to_screen(aim->aim_point);
        ASSERT_TRUE(crosshair.has_value());
        EXPECT_NEAR(crosshair->x, 960.f, 0.5f);
        EXPECT_NEAR(crosshair->y, 540.f, 0.5f);

        // The target itself is not under the crosshair: the round leaves above the view, so the target shows up above
        // the centre of the screen (smaller y with a top-left origin).
        const auto target_on_screen = camera.world_to_screen(aim->predicted_target_position);
        ASSERT_TRUE(target_on_screen.has_value());
        EXPECT_LT(target_on_screen->y, 540.f - 20.f);

        // look_at() towards the aim point is the other way to get the same view
        const auto looked_at = CameraTrait::calc_look_at_angle(launcher.eye_origin, aim->aim_point);
        EXPECT_NEAR(looked_at.pitch.as_degrees(), -aim->angles.pitch, 0.01f);
        EXPECT_NEAR(looked_at.yaw.as_degrees(), aim->angles.yaw, 0.01f);
    }
} // namespace
