#include <cmath>
#include <gtest/gtest.h>
#include <omath/engines/source_engine/traits/camera_trait.hpp>
#include <omath/engines/source_engine/traits/pred_engine_trait.hpp>
#include <omath/projectile_prediction/proj_pred_engine_avx2.hpp>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>
#include <stdexcept>

using Projectile = omath::projectile_prediction::Projectile<float>;
using Target = omath::projectile_prediction::Target<float>;
using Launcher = omath::projectile_prediction::Launcher<float>;
using Avx2Engine = omath::projectile_prediction::ProjPredEngineAvx2;

#if defined(OMATH_USE_AVX2) && (defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86))

// Fire from where the solution says the muzzle is, with the solution's angles, and see how close the projectile gets
// to the predicted target at the solution's time of flight. The AVX2 solver picks the first feasible time step, so the
// miss can be up to one step of travel.
static void expect_avx2_solution_hits(const Projectile& proj, const Launcher& launcher, const Target& target,
                                      const float gravity, const float step, const float max_time,
                                      const float hit_tolerance)
{
    using Trait = omath::source_engine::PredEngineTrait;
    const Avx2Engine engine(gravity, step, max_time);

    const auto solution = engine.maybe_calculate_aim(proj, launcher, target);
    ASSERT_TRUE(solution.has_value()) << "AVX2 engine must find a solution";

    // The muzzle turns with the view; the round leaves along the view raised by the launcher's pitch offset
    const auto& [pitch, yaw] = solution->angles;
    const auto launch_origin = launcher.launch_origin(Trait::calc_view_basis(pitch, yaw));
    const auto proj_pos = Trait::predict_projectile_position(launch_origin, proj, pitch + launcher.launch_pitch_offset,
                                                             yaw, solution->time_of_flight, gravity);
    const auto tgt_pos = Trait::predict_target_position(target, solution->time_of_flight, gravity);

    EXPECT_LE(proj_pos.distance_to(tgt_pos), hit_tolerance)
            << "closest approach " << proj_pos.distance_to(tgt_pos) << " at t=" << solution->time_of_flight;

    const auto [cam_pitch, cam_yaw, cam_roll] =
            omath::source_engine::CameraTrait::calc_look_at_angle(launcher.eye_origin, solution->aim_point);
    EXPECT_NEAR(pitch, -cam_pitch.as_degrees(), 0.01f);
    EXPECT_NEAR(yaw, cam_yaw.as_degrees(), 0.01f);
}

TEST(ProjPredEngineAvx2, StaticTarget)
{
    constexpr Target target{.m_origin = {300, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 2000.f, .m_gravity_scale = 0.4f};
    constexpr Launcher launcher{.eye_origin = {3, 2, 1}};

    expect_avx2_solution_hits(proj, launcher, target, 400.f, 1.f / 1000.f, 50.f, 5.f);
}

TEST(ProjPredEngineAvx2, MovingTarget)
{
    constexpr Target target{.m_origin = {500, 100, 0}, .m_velocity = {-50, 20, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 1500.f, .m_gravity_scale = 1.0f};
    constexpr Launcher launcher{.eye_origin = {0, 0, 64}};

    expect_avx2_solution_hits(proj, launcher, target, 800.f, 1.f / 1000.f, 30.f, 5.f);
}

TEST(ProjPredEngineAvx2, AirborneTargetWithMuzzleOffset)
{
    constexpr Target target{.m_origin = {200, 50, 300}, .m_velocity = {10, -5, -20}, .m_is_airborne = true};
    constexpr Projectile proj = {.m_launch_speed = 1800.f, .m_gravity_scale = 0.5f};
    constexpr Launcher launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = {.forward = 16.f, .right = 8.f, .up = -6.f}};

    expect_avx2_solution_hits(proj, launcher, target, 400.f, 1.f / 1000.f, 50.f, 6.f);
}

TEST(ProjPredEngineAvx2, AgreesWithLegacyEngine)
{
    constexpr Target target{.m_origin = {400, -150, 30}, .m_velocity = {-30, 10, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 2500.f, .m_gravity_scale = 0.6f};
    constexpr Launcher launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = {.forward = 16.f, .right = 8.f, .up = -6.f}};

    const auto avx2 = Avx2Engine(400.f, 1.f / 1000.f, 50.f).maybe_calculate_aim(proj, launcher, target);
    const auto legacy = omath::projectile_prediction::ProjPredEngineLegacy<>(400.f, 1.f / 1000.f, 50.f, 5.f)
                                .maybe_calculate_aim(proj, launcher, target);
    ASSERT_TRUE(avx2.has_value());
    ASSERT_TRUE(legacy.has_value());

    EXPECT_NEAR(avx2->angles.pitch, legacy->angles.pitch, 0.25f);
    EXPECT_NEAR(avx2->angles.yaw, legacy->angles.yaw, 0.25f);
    EXPECT_NEAR(avx2->time_of_flight, legacy->time_of_flight, 3.f / 1000.f);
}

TEST(ProjPredEngineAvx2, PipeLauncherWithPitchOffset)
{
    // TF2-style: velocity = forward * 1200 + up * 200 in the view frame
    const float pitch_offset = omath::angles::radians_to_degrees(std::atan2(200.f, 1200.f));
    constexpr Target target{.m_origin = {600, 150, 0}, .m_velocity = {-40, 30, 0}, .m_is_airborne = false};
    const Projectile proj = {.m_launch_speed = std::hypot(1200.f, 200.f), .m_gravity_scale = 0.5f};
    const Launcher launcher{.eye_origin = {0, 0, 64},
                            .muzzle_offset = {.forward = 16.f, .right = 8.f, .up = -6.f},
                            .launch_pitch_offset = pitch_offset};

    expect_avx2_solution_hits(proj, launcher, target, 800.f, 1.f / 1000.f, 5.f, 5.f);

    // Same shot through the legacy engine: the view pitch must carry the same offset
    const auto avx2 = Avx2Engine(800.f, 1.f / 1000.f, 5.f).maybe_calculate_aim(proj, launcher, target);
    const auto legacy = omath::projectile_prediction::ProjPredEngineLegacy<>(800.f, 1.f / 1000.f, 5.f, 5.f)
                                .maybe_calculate_aim(proj, launcher, target);
    ASSERT_TRUE(avx2.has_value());
    ASSERT_TRUE(legacy.has_value());
    EXPECT_NEAR(avx2->angles.pitch, legacy->angles.pitch, 0.25f);
    EXPECT_NEAR(avx2->angles.yaw, legacy->angles.yaw, 0.25f);
}

TEST(ProjPredEngineAvx2, UnreachableViewPitchIsRejected)
{
    const float pitch_offset = omath::angles::radians_to_degrees(std::atan2(200.f, 1200.f));
    constexpr Target below{.m_origin = {30, 0, 0}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const Projectile proj = {.m_launch_speed = std::hypot(1200.f, 200.f), .m_gravity_scale = 0.5f};
    const Avx2Engine engine(800.f, 1.f / 1000.f, 5.f);

    EXPECT_TRUE(engine.maybe_calculate_aim(proj, Launcher{.eye_origin = {0, 0, 1000}}, below).has_value());
    EXPECT_FALSE(engine.maybe_calculate_aim(
                               proj, Launcher{.eye_origin = {0, 0, 1000}, .launch_pitch_offset = pitch_offset}, below)
                         .has_value());
}

TEST(ProjPredEngineAvx2, GroundedTargetIsNotPulledDownByTheScan)
{
    // Regression: the vectorised feasibility scan used to apply gravity to every target, airborne or not. With a round
    // that falls slower than the target supposedly did, the scan threw the true flight time away and accepted a later
    // one, for which the solved pitch belongs to a slower round. Over a flight this long that is a miss of hundreds of
    // units.
    constexpr Target target{.m_origin = {2500, 400, 0}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 1500.f, .m_gravity_scale = 0.2f};
    constexpr Launcher launcher{.eye_origin = {0, 0, 64}};

    expect_avx2_solution_hits(proj, launcher, target, 800.f, 1.f / 1000.f, 10.f, 6.f);

    const auto avx2 = Avx2Engine(800.f, 1.f / 1000.f, 10.f).maybe_calculate_aim(proj, launcher, target);
    const auto legacy = omath::projectile_prediction::ProjPredEngineLegacy<>(800.f, 1.f / 1000.f, 10.f, 5.f)
                                .maybe_calculate_aim(proj, launcher, target);
    ASSERT_TRUE(avx2.has_value());
    ASSERT_TRUE(legacy.has_value());
    EXPECT_NEAR(avx2->angles.pitch, legacy->angles.pitch, 0.25f);
    EXPECT_NEAR(avx2->angles.yaw, legacy->angles.yaw, 0.25f);
    EXPECT_NEAR(avx2->time_of_flight, legacy->time_of_flight, 5.f / 1000.f);
}

TEST(ProjPredEngineAvx2, AirborneTargetFallsDuringTheFlight)
{
    // The same target, airborne or not, must be aimed at differently: lower when it is falling
    constexpr Projectile proj = {.m_launch_speed = 1500.f, .m_gravity_scale = 0.2f};
    constexpr Launcher launcher{.eye_origin = {0, 0, 64}};
    constexpr Target grounded{.m_origin = {1500, 0, 600}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Target falling{.m_origin = {1500, 0, 600}, .m_velocity = {0, 0, 0}, .m_is_airborne = true};

    const Avx2Engine engine(800.f, 1.f / 1000.f, 10.f);
    const auto at_grounded = engine.maybe_calculate_aim(proj, launcher, grounded);
    const auto at_falling = engine.maybe_calculate_aim(proj, launcher, falling);
    ASSERT_TRUE(at_grounded.has_value());
    ASSERT_TRUE(at_falling.has_value());

    EXPECT_LT(at_falling->angles.pitch, at_grounded->angles.pitch - 5.f);
    EXPECT_LT(at_falling->predicted_target_position.z, 600.f - 100.f);
    EXPECT_FLOAT_EQ(at_grounded->predicted_target_position.z, 600.f);

    expect_avx2_solution_hits(proj, launcher, falling, 800.f, 1.f / 1000.f, 10.f, 6.f);
}

TEST(ProjPredEngineAvx2, AHorizonShorterThanTheFlightFindsNothing)
{
    constexpr Target target{.m_origin = {3000, 0, 64}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 1500.f, .m_gravity_scale = 0.5f};
    constexpr Launcher launcher{.eye_origin = {0, 0, 64}};

    EXPECT_FALSE(Avx2Engine(800.f, 1.f / 1000.f, 1.5f).maybe_calculate_aim(proj, launcher, target).has_value());

    const auto aim = Avx2Engine(800.f, 1.f / 1000.f, 3.f).maybe_calculate_aim(proj, launcher, target);
    ASSERT_TRUE(aim.has_value());
    EXPECT_GT(aim->time_of_flight, 1.9f);
    EXPECT_LT(aim->time_of_flight, 2.3f);
}

TEST(ProjPredEngineAvx2, WorldOffsetMovesTheLaunchOrigin)
{
    // Firing from 40 units lower has to aim higher to reach the same spot
    constexpr Target target{.m_origin = {900, 100, 64}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 1500.f, .m_gravity_scale = 0.5f};
    constexpr Launcher from_the_eye{.eye_origin = {0, 0, 64}};
    constexpr Launcher from_below{.eye_origin = {0, 0, 64}, .world_offset = {0, 0, -40}};

    const Avx2Engine engine(800.f, 1.f / 1000.f, 10.f);
    const auto high = engine.maybe_calculate_aim(proj, from_the_eye, target);
    const auto low = engine.maybe_calculate_aim(proj, from_below, target);
    ASSERT_TRUE(high.has_value());
    ASSERT_TRUE(low.has_value());
    EXPECT_GT(low->angles.pitch, high->angles.pitch + 1.f);

    expect_avx2_solution_hits(proj, from_below, target, 800.f, 1.f / 1000.f, 10.f, 5.f);
}

TEST(ProjPredEngineAvx2, WrappersAndTheInterfaceGiveTheSameAnswer)
{
    constexpr Target target{.m_origin = {500, 100, 0}, .m_velocity = {-50, 20, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {
            .m_origin = {0, 0, 64}, .m_launch_offset = {5, 0, -3}, .m_launch_speed = 1500.f, .m_gravity_scale = 1.f};

    const Avx2Engine engine(800.f, 1.f / 1000.f, 10.f);
    const omath::projectile_prediction::ProjPredEngineInterface<float>& through_base = engine;

    const auto solution = engine.maybe_calculate_aim(proj, Avx2Engine::launcher_from_projectile(proj), target);
    const auto dispatched = through_base.maybe_calculate_aim(proj, Avx2Engine::launcher_from_projectile(proj), target);
    const auto point = engine.maybe_calculate_aim_point(proj, target);
    const auto angles = through_base.maybe_calculate_aim_angles(proj, target);
    ASSERT_TRUE(solution.has_value());
    ASSERT_TRUE(dispatched.has_value());
    ASSERT_TRUE(point.has_value());
    ASSERT_TRUE(angles.has_value());

    EXPECT_EQ(solution->aim_point, dispatched->aim_point);
    EXPECT_EQ(solution->aim_point, point.value());
    EXPECT_FLOAT_EQ(solution->angles.pitch, angles->pitch);
    EXPECT_FLOAT_EQ(solution->angles.yaw, angles->yaw);
}

TEST(ProjPredEngineAvx2, OutOfRangeReturnsNullopt)
{
    constexpr Target target{.m_origin = {100000, 0, 0}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 1.f, .m_gravity_scale = 1.f};

    const Avx2Engine engine(400.f, 1.f / 1000.f, 5.f);
    EXPECT_FALSE(engine.maybe_calculate_aim(proj, Launcher{.eye_origin = {0, 0, 0}}, target).has_value());
    EXPECT_FALSE(engine.maybe_calculate_aim_point(proj, target).has_value());
}

#else

TEST(ProjPredEngineAvx2, ThrowsWhenAvx2IsNotCompiledIn)
{
    constexpr Target target{.m_origin = {300, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 2000.f, .m_gravity_scale = 0.4f};

    const Avx2Engine engine(400.f, 1.f / 1000.f, 50.f);
    EXPECT_THROW(std::ignore = engine.maybe_calculate_aim(proj, Launcher{.eye_origin = {3, 2, 1}}, target),
                 std::runtime_error);
    EXPECT_THROW(std::ignore = engine.maybe_calculate_aim_point(proj, target), std::runtime_error);
}

#endif
