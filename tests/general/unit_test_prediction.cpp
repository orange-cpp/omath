#include <cmath>
#include <gtest/gtest.h>
#include <numbers>
#include <omath/engines/source_engine/traits/camera_trait.hpp>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>
#include <type_traits>

using Projectile = omath::projectile_prediction::Projectile<float>;
using Target = omath::projectile_prediction::Target<float>;
using Engine = omath::projectile_prediction::ProjPredEngineLegacy<>;

TEST(UnitTestPrediction, PredictionTest)
{
    constexpr Target target{.m_origin = {100, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {3, 2, 1}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};
    const auto viewPoint = Engine(400.f, 1.f / 1000.f, 50.f, 5.f).maybe_calculate_aim_point(proj, target);

    const auto [pitch, yaw, _] =
            omath::source_engine::CameraTrait::calc_look_at_angle(proj.m_origin, viewPoint.value());

    EXPECT_NEAR(-42.547142, pitch.as_degrees(), 0.01f);
    EXPECT_NEAR(-1.181189, yaw.as_degrees(), 0.01f);
}

// Helper: verify aim_angles match angles derived from aim_point via CameraTrait
static void expect_angles_match_aim_point(const Projectile& proj, const Target& target, float gravity, float step,
                                          float max_time, float tolerance, float angle_eps = 0.01f)
{
    const Engine engine(gravity, step, max_time, tolerance);

    const auto aim_point = engine.maybe_calculate_aim_point(proj, target);
    const auto aim_angles = engine.maybe_calculate_aim_angles(proj, target);

    ASSERT_TRUE(aim_point.has_value()) << "aim_point should have a solution";
    ASSERT_TRUE(aim_angles.has_value()) << "aim_angles should have a solution";

    // Source engine CameraTrait: pitch = -asin(dir.z), yaw = atan2(dir.y, dir.x)
    // PredEngineTrait: pitch = asin(delta.z / dist), yaw = atan2(delta.y, delta.x)
    // So aim_angles.pitch == -camera_pitch, aim_angles.yaw == camera_yaw
    const auto [cam_pitch, cam_yaw, cam_roll] =
            omath::source_engine::CameraTrait::calc_look_at_angle(proj.m_origin, aim_point.value());

    EXPECT_NEAR(aim_angles->pitch, -cam_pitch.as_degrees(), angle_eps)
            << "pitch from aim_angles must match pitch derived from aim_point";
    EXPECT_NEAR(aim_angles->yaw, cam_yaw.as_degrees(), angle_eps)
            << "yaw from aim_angles must match yaw derived from aim_point";
}

TEST(UnitTestPrediction, AimAnglesMatchAimPoint_StaticTarget)
{
    constexpr Target target{.m_origin = {100, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {3, 2, 1}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    expect_angles_match_aim_point(proj, target, 400, 1.f / 1000.f, 50, 5.f);
}

TEST(UnitTestPrediction, AimAnglesMatchAimPoint_MovingTarget)
{
    constexpr Target target{.m_origin = {500, 100, 0}, .m_velocity = {-50, 20, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {0, 0, 0}, .m_launch_speed = 3000.f, .m_gravity_scale = 1.0f};

    expect_angles_match_aim_point(proj, target, 800, 1.f / 500.f, 30, 10.f);
}

TEST(UnitTestPrediction, AimAnglesMatchAimPoint_AirborneTarget)
{
    constexpr Target target{.m_origin = {200, 50, 300}, .m_velocity = {10, -5, -20}, .m_is_airborne = true};
    constexpr Projectile proj = {.m_origin = {0, 0, 0}, .m_launch_speed = 4000.f, .m_gravity_scale = 0.5f};

    expect_angles_match_aim_point(proj, target, 400, 1.f / 1000.f, 50, 10.f);
}

TEST(UnitTestPrediction, AimAnglesMatchAimPoint_HighArc)
{
    // Target nearly directly above — high pitch angle
    constexpr Target target{.m_origin = {10, 0, 500}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {0, 0, 0}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.3f};

    expect_angles_match_aim_point(proj, target, 400, 1.f / 1000.f, 50, 5.f);
}

TEST(UnitTestPrediction, AimAnglesMatchAimPoint_NegativeYaw)
{
    // Target behind and to the left — negative yaw quadrant
    constexpr Target target{.m_origin = {-200, -150, 10}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {0, 0, 0}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    expect_angles_match_aim_point(proj, target, 400, 1.f / 1000.f, 50, 5.f);
}

TEST(UnitTestPrediction, AimAnglesMatchAimPoint_WithLaunchOffset)
{
    constexpr Target target{.m_origin = {200, 0, 50}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_offset = {5, 0, -3}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    expect_angles_match_aim_point(proj, target, 400, 1.f / 1000.f, 50, 5.f);
}

// Helper: simulate projectile flight using aim_angles and verify it reaches the target.
// Steps the projectile forward in small increments, simultaneously predicts target position,
// and checks that the minimum distance is within hit_tolerance.
static void expect_projectile_hits_target(const Projectile& proj, const Target& target, float gravity,
                                          float engine_step, float max_time, float engine_tolerance,
                                          float hit_tolerance, float sim_step = 1.f / 2000.f)
{
    using Trait = omath::source_engine::PredEngineTrait;
    const Engine engine(gravity, engine_step, max_time, engine_tolerance);

    const auto aim_angles = engine.maybe_calculate_aim_angles(proj, target);
    ASSERT_TRUE(aim_angles.has_value()) << "engine must find a solution";

    float min_dist = std::numeric_limits<float>::max();
    float best_time = 0.f;

    for (float t = 0.f; t <= max_time; t += sim_step)
    {
        const auto proj_pos = Trait::predict_projectile_position(proj.m_origin + proj.m_launch_offset, proj,
                                                                 aim_angles->pitch, aim_angles->yaw, t, gravity);
        const auto tgt_pos = Trait::predict_target_position(target, t, gravity);
        const float dist = proj_pos.distance_to(tgt_pos);

        if (dist < min_dist)
        {
            min_dist = dist;
            best_time = t;
        }

        // Early exit once distance starts increasing significantly after approaching
        if (dist > min_dist + hit_tolerance * 10.f && min_dist < hit_tolerance * 100.f)
            break;
    }

    EXPECT_LE(min_dist, hit_tolerance) << "Projectile must reach target. Closest approach: " << min_dist
                                       << " at t=" << best_time;
}

// ── Simulation hit tests: no launch offset ─────────────────────────────────

TEST(ProjectileSimulation, HitsStaticTarget_NoOffset)
{
    constexpr Target target{.m_origin = {100, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {3, 2, 1}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    expect_projectile_hits_target(proj, target, 400, 1.f / 1000.f, 50, 5.f, 10.f);
}

TEST(ProjectileSimulation, HitsMovingTarget_NoOffset)
{
    constexpr Target target{.m_origin = {500, 100, 0}, .m_velocity = {-50, 20, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {0, 0, 0}, .m_launch_speed = 3000.f, .m_gravity_scale = 1.0f};

    expect_projectile_hits_target(proj, target, 800, 1.f / 500.f, 30, 10.f, 15.f);
}

TEST(ProjectileSimulation, HitsAirborneTarget_NoOffset)
{
    constexpr Target target{.m_origin = {200, 50, 300}, .m_velocity = {10, -5, -20}, .m_is_airborne = true};
    constexpr Projectile proj = {.m_origin = {0, 0, 0}, .m_launch_speed = 4000.f, .m_gravity_scale = 0.5f};

    expect_projectile_hits_target(proj, target, 400, 1.f / 1000.f, 50, 10.f, 15.f);
}

TEST(ProjectileSimulation, HitsHighTarget_NoOffset)
{
    constexpr Target target{.m_origin = {10, 0, 500}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {0, 0, 0}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.3f};

    expect_projectile_hits_target(proj, target, 400, 1.f / 1000.f, 50, 5.f, 10.f);
}

TEST(ProjectileSimulation, HitsNegativeYawTarget_NoOffset)
{
    constexpr Target target{.m_origin = {-200, -150, 10}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {0, 0, 0}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    expect_projectile_hits_target(proj, target, 400, 1.f / 1000.f, 50, 5.f, 10.f);
}

// ── Simulation hit tests: with launch offset ────────────────────────────────

TEST(ProjectileSimulation, HitsStaticTarget_SmallOffset)
{
    constexpr Target target{.m_origin = {200, 0, 50}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_offset = {5, 0, -3}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    expect_projectile_hits_target(proj, target, 400, 1.f / 1000.f, 50, 5.f, 10.f);
}

TEST(ProjectileSimulation, HitsStaticTarget_LargeXOffset)
{
    constexpr Target target{.m_origin = {300, 100, 0}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_offset = {20, 0, 0}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    expect_projectile_hits_target(proj, target, 400, 1.f / 1000.f, 50, 5.f, 10.f);
}

TEST(ProjectileSimulation, HitsStaticTarget_LargeYOffset)
{
    constexpr Target target{.m_origin = {150, -200, 30}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_offset = {0, 15, 0}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    expect_projectile_hits_target(proj, target, 400, 1.f / 1000.f, 50, 5.f, 10.f);
}

TEST(ProjectileSimulation, HitsStaticTarget_LargeZOffset)
{
    constexpr Target target{.m_origin = {100, 0, 200}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_offset = {0, 0, -10}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    expect_projectile_hits_target(proj, target, 400, 1.f / 1000.f, 50, 5.f, 10.f);
}

TEST(ProjectileSimulation, HitsStaticTarget_AllAxesOffset)
{
    constexpr Target target{.m_origin = {250, 80, 60}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const Projectile proj = {
            .m_origin = {10, 5, 20}, .m_launch_offset = {8, -4, -6}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    expect_projectile_hits_target(proj, target, 400, 1.f / 1000.f, 50, 5.f, 10.f);
}

TEST(ProjectileSimulation, HitsMovingTarget_WithOffset)
{
    constexpr Target target{.m_origin = {400, 0, 50}, .m_velocity = {-30, 10, 5}, .m_is_airborne = false};
    const Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_offset = {10, -5, 2}, .m_launch_speed = 3000.f, .m_gravity_scale = 0.8f};

    expect_projectile_hits_target(proj, target, 800, 1.f / 500.f, 30, 10.f, 15.f);
}

TEST(ProjectileSimulation, HitsAirborneTarget_WithOffset)
{
    constexpr Target target{.m_origin = {150, 80, 250}, .m_velocity = {5, -10, -30}, .m_is_airborne = true};
    const Projectile proj = {
            .m_origin = {0, 0, 50}, .m_launch_offset = {3, 7, -5}, .m_launch_speed = 4000.f, .m_gravity_scale = 0.5f};

    expect_projectile_hits_target(proj, target, 400, 1.f / 1000.f, 50, 10.f, 15.f);
}

TEST(ProjectileSimulation, HitsNegativeYawTarget_WithOffset)
{
    constexpr Target target{.m_origin = {-200, -150, 10}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_offset = {-5, 3, 2}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    expect_projectile_hits_target(proj, target, 400, 1.f / 1000.f, 50, 5.f, 10.f);
}

TEST(UnitTestPrediction, AimAnglesReturnsNulloptWhenNoSolution)
{
    constexpr Target target{.m_origin = {100000, 0, 0}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {0, 0, 0}, .m_launch_speed = 1.f, .m_gravity_scale = 1.f};

    const Engine engine(9.81f, 0.1f, 2.f, 5.f);

    const auto aim_point = engine.maybe_calculate_aim_point(proj, target);
    const auto aim_angles = engine.maybe_calculate_aim_angles(proj, target);

    EXPECT_FALSE(aim_point.has_value());
    EXPECT_FALSE(aim_angles.has_value());
}

// Regression tests for the legacy engine cleanup: fixed step count, pitch against a double reference, vertical shots.
TEST(UnitTestPrediction, ZeroTimeStepReturnsNulloptInsteadOfHanging)
{
    constexpr Target target{.m_origin = {100, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {3, 2, 1}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    EXPECT_FALSE(Engine(400.f, 0.f, 50.f, 5.f).maybe_calculate_aim_point(proj, target).has_value());
    EXPECT_FALSE(Engine(400.f, -1.f, 50.f, 5.f).maybe_calculate_aim_angles(proj, target).has_value());
    EXPECT_FALSE(Engine(400.f, 1.f / 1000.f, 0.f, 5.f).maybe_calculate_aim_point(proj, target).has_value());
}

TEST(UnitTestPrediction, HighSpeedPitchMatchesDoubleReference)
{
    // Static target, so the solved pitch is the closed form for a fixed (horizontal distance, height) pair.
    constexpr Target target{.m_origin = {100, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {3, 2, 1}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    const auto aim_angles = Engine(400.f, 1.f / 1000.f, 50.f, 5.f).maybe_calculate_aim_angles(proj, target);
    ASSERT_TRUE(aim_angles.has_value());

    const double v = 5000.0;
    const double g = 400.0 * 0.4;
    const double x = std::hypot(100.0 - 3.0, 0.0 - 2.0);
    const double y = 90.0 - 1.0;
    const double reference = std::atan((v * v - std::sqrt(v * v * v * v - g * (g * x * x + 2.0 * y * v * v))) / (g * x))
                             * 180.0 / std::numbers::pi;

    // At this speed v^2 and sqrt(D) are nearly equal and their difference keeps few float digits: the pitch comes out
    // about 0.002 degrees off the double reference, which over these 97 units is 0.003 of a unit.
    EXPECT_NEAR(aim_angles->pitch, static_cast<float>(reference), 1e-2f);
}

TEST(UnitTestPrediction, HighArcReachesTheTargetSteeperAndLater)
{
    using omath::projectile_prediction::Arc;

    constexpr Target target{.m_origin = {800, 100, 64}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {0, 0, 64}, .m_launch_speed = 1200.f, .m_gravity_scale = 1.f};

    const auto low = Engine(800.f, 1.f / 1000.f, 5.f, 5.f, Arc::LOW).maybe_calculate_aim_angles(proj, target);
    const auto high = Engine(800.f, 1.f / 1000.f, 5.f, 5.f, Arc::HIGH).maybe_calculate_aim_angles(proj, target);
    ASSERT_TRUE(low.has_value());
    ASSERT_TRUE(high.has_value());

    // The two roots of the same quadratic: their tangents multiply to 2 y v^2 / (g x^2) + 1, which is 1 on the level
    EXPECT_NEAR(std::tan(low->pitch * std::numbers::pi_v<float> / 180.f)
                        * std::tan(high->pitch * std::numbers::pi_v<float> / 180.f),
                1.f, 1e-2f);
    EXPECT_GT(high->pitch, 45.f);
    EXPECT_LT(low->pitch, 45.f);
    EXPECT_NEAR(high->yaw, low->yaw, 1e-3f);
}

TEST(UnitTestPrediction, WithoutGravityTheArcsAgree)
{
    using omath::projectile_prediction::Arc;

    constexpr Target target{.m_origin = {800, 100, 200}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_origin = {0, 0, 64}, .m_launch_speed = 1100.f, .m_gravity_scale = 0.f};

    const auto low = Engine(800.f, 1.f / 1000.f, 5.f, 5.f, Arc::LOW).maybe_calculate_aim_angles(proj, target);
    const auto high = Engine(800.f, 1.f / 1000.f, 5.f, 5.f, Arc::HIGH).maybe_calculate_aim_angles(proj, target);
    ASSERT_TRUE(low.has_value());
    ASSERT_TRUE(high.has_value());

    EXPECT_NEAR(high->pitch, low->pitch, 1e-4f);
    EXPECT_NEAR(high->yaw, low->yaw, 1e-4f);
}

TEST(UnitTestPrediction, VerticalShotsUseDirectPitch)
{
    constexpr Projectile proj = {.m_origin = {0, 0, 0}, .m_launch_speed = 500.f, .m_gravity_scale = 1.f};
    const Engine engine(400.f, 1.f / 1000.f, 5.f, 5.f);

    constexpr Target above{.m_origin = {0, 0, 100}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const auto up = engine.maybe_calculate_aim_angles(proj, above);
    ASSERT_TRUE(up.has_value());
    EXPECT_FLOAT_EQ(up->pitch, 90.f);

    constexpr Target below{.m_origin = {0, 0, -100}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const auto down = engine.maybe_calculate_aim_angles(proj, below);
    ASSERT_TRUE(down.has_value());
    EXPECT_FLOAT_EQ(down->pitch, -90.f);
}

TEST(UnitTestPrediction, AimPointAndAnglesAgreeWithLateralOffset)
{
    // The aim point lies on the eye's aim ray, so the camera angles towards it are the launch angles even when the
    // muzzle sits 50 units to the side and fires along a different bearing than the camera-to-target line.
    constexpr Target target{.m_origin = {200, 0, 0}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_offset = {0, 50, 0}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    const Engine engine(400.f, 1.f / 1000.f, 50.f, 5.f);
    const auto aim_angles = engine.maybe_calculate_aim_angles(proj, target);
    const auto aim_point = engine.maybe_calculate_aim_point(proj, target);
    ASSERT_TRUE(aim_angles.has_value());
    ASSERT_TRUE(aim_point.has_value());

    const float expected_yaw = omath::angles::radians_to_degrees(std::atan2(0.f - 50.f, 200.f - 0.f));
    EXPECT_NEAR(aim_angles->yaw, expected_yaw, 0.05f);

    const auto [cam_pitch, cam_yaw, cam_roll] =
            omath::source_engine::CameraTrait::calc_look_at_angle(proj.m_origin, aim_point.value());
    EXPECT_NEAR(aim_angles->pitch, -cam_pitch.as_degrees(), 0.01f);
    EXPECT_NEAR(aim_angles->yaw, cam_yaw.as_degrees(), 0.01f);
}

TEST(UnitTestPrediction, EngineIsAssignable)
{
    static_assert(std::is_copy_assignable_v<Engine>);
    static_assert(std::is_nothrow_constructible_v<Engine, float, float, float, float>);
    SUCCEED();
}

// ---- The Launcher API: view-relative muzzle offset ----

using Launcher = omath::projectile_prediction::Launcher<float>;

// The projectile leaves from eye + basis(angles) * offset, so the returned angles have to hit from exactly there.
// Simulate that with the trait at the solution's own time of flight.
static void expect_launcher_solution_hits(const Projectile& proj, const Launcher& launcher, const Target& target,
                                          float gravity, float engine_step, float max_time, float engine_tolerance,
                                          float hit_tolerance)
{
    using Trait = omath::source_engine::PredEngineTrait;
    const Engine engine(gravity, engine_step, max_time, engine_tolerance);

    const auto solution = engine.maybe_calculate_aim(proj, launcher, target);
    ASSERT_TRUE(solution.has_value()) << "engine must find a solution";

    // The muzzle turns with the view; the round leaves along the view raised by the launcher's pitch offset
    const auto& [pitch, yaw] = solution->angles;
    const auto launch_origin = launcher.launch_origin(Trait::calc_view_basis(pitch, yaw));
    const auto proj_pos = Trait::predict_projectile_position(launch_origin, proj, pitch + launcher.launch_pitch_offset,
                                                             yaw, solution->time_of_flight, gravity);
    const auto tgt_pos = Trait::predict_target_position(target, solution->time_of_flight, gravity);

    EXPECT_LE(proj_pos.distance_to(tgt_pos), hit_tolerance)
            << "projectile fired from the rotated muzzle must reach the target";
    EXPECT_NEAR(solution->predicted_target_position.x, tgt_pos.x, 1e-3f);
    EXPECT_NEAR(solution->predicted_target_position.y, tgt_pos.y, 1e-3f);
    EXPECT_NEAR(solution->predicted_target_position.z, tgt_pos.z, 1e-3f);

    // The aim point sits on the eye's aim ray
    const auto [cam_pitch, cam_yaw, cam_roll] =
            omath::source_engine::CameraTrait::calc_look_at_angle(launcher.eye_origin, solution->aim_point);
    EXPECT_NEAR(pitch, -cam_pitch.as_degrees(), 0.01f);
    EXPECT_NEAR(yaw, cam_yaw.as_degrees(), 0.01f);
}

// Source-style spawn: 16 forward, 8 to the right, 6 down from the eye
constexpr omath::projectile_prediction::MuzzleOffset<float> k_source_muzzle{.forward = 16.f, .right = 8.f, .up = -6.f};

TEST(UnitTestPredictionLauncher, StaticTargetWithMuzzleOffset)
{
    constexpr Target target{.m_origin = {200, 0, 50}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};
    constexpr Launcher launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = k_source_muzzle};

    expect_launcher_solution_hits(proj, launcher, target, 400, 1.f / 1000.f, 50, 5.f, 10.f);
}

TEST(UnitTestPredictionLauncher, MovingTargetWithMuzzleOffset)
{
    constexpr Target target{.m_origin = {500, 100, 0}, .m_velocity = {-50, 20, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 3000.f, .m_gravity_scale = 1.0f};
    constexpr Launcher launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = k_source_muzzle};

    expect_launcher_solution_hits(proj, launcher, target, 800, 1.f / 500.f, 30, 10.f, 15.f);
}

TEST(UnitTestPredictionLauncher, SideTargetWithMuzzleOffset)
{
    // Target off to the left so the rotated muzzle ends up well away from where a world-space offset would put it
    constexpr Target target{.m_origin = {-50, 300, 80}, .m_velocity = {10, -5, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 2500.f, .m_gravity_scale = 1.0f};
    constexpr Launcher launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = k_source_muzzle};

    expect_launcher_solution_hits(proj, launcher, target, 800, 1.f / 1000.f, 30, 10.f, 15.f);
}

TEST(UnitTestPredictionLauncher, WorldAndViewOffsetsCombine)
{
    constexpr Target target{.m_origin = {300, -120, 20}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 4000.f, .m_gravity_scale = 0.5f};
    constexpr Launcher launcher{
            .eye_origin = {10, 5, 64}, .muzzle_offset = k_source_muzzle, .world_offset = {0, 0, -10}};

    expect_launcher_solution_hits(proj, launcher, target, 400, 1.f / 1000.f, 50, 5.f, 10.f);
}

TEST(UnitTestPredictionLauncher, MuzzleOffsetChangesTheAnswer)
{
    // A muzzle 8 units to the right must fire on a different bearing than one at the eye
    constexpr Target target{.m_origin = {300, 0, 64}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};
    const Engine engine(400.f, 1.f / 1000.f, 50.f, 5.f);

    const auto centred = engine.maybe_calculate_aim(proj, Launcher{.eye_origin = {0, 0, 64}}, target);
    const auto offset = engine.maybe_calculate_aim(
            proj, Launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = {.forward = 0.f, .right = 8.f, .up = 0.f}},
            target);
    ASSERT_TRUE(centred.has_value());
    ASSERT_TRUE(offset.has_value());

    // Source right is -Y at zero yaw, so the muzzle sits at y = -8 and must yaw towards +Y by atan(8 / 300)
    const float expected_shift = omath::angles::radians_to_degrees(std::atan2(8.f, 300.f));
    EXPECT_NEAR(centred->angles.yaw, 0.f, 0.05f);
    EXPECT_NEAR(offset->angles.yaw - centred->angles.yaw, expected_shift, 0.05f);
}

TEST(UnitTestPredictionLauncher, WrappersMatchLauncherWithWorldOffset)
{
    constexpr Target target{.m_origin = {200, 0, 50}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {
            .m_origin = {0, 0, 64}, .m_launch_offset = {5, 0, -3}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};
    const Engine engine(400.f, 1.f / 1000.f, 50.f, 5.f);

    const auto solution = engine.maybe_calculate_aim(proj, Engine::launcher_from_projectile(proj), target);
    const auto point = engine.maybe_calculate_aim_point(proj, target);
    const auto angles = engine.maybe_calculate_aim_angles(proj, target);
    ASSERT_TRUE(solution.has_value());
    ASSERT_TRUE(point.has_value());
    ASSERT_TRUE(angles.has_value());

    EXPECT_EQ(solution->aim_point, point.value());
    EXPECT_FLOAT_EQ(solution->angles.pitch, angles->pitch);
    EXPECT_FLOAT_EQ(solution->angles.yaw, angles->yaw);
}

// ---- Launch pitch offset: weapons that fire above the crosshair, such as TF2's pipe and sticky launchers ----

// TF2 sets a pipe's velocity to forward * 1200 + up * 200 in the view frame
static float pipe_speed()
{
    return std::hypot(1200.f, 200.f);
}
static float pipe_pitch_offset()
{
    return omath::angles::radians_to_degrees(std::atan2(200.f, 1200.f));
}

TEST(UnitTestPredictionLauncher, PitchOffsetIsForwardPlusUpVelocity)
{
    // The equivalence the feature rests on: forward * a + up * b in the view frame is the view direction pitched up by
    // atan(b / a), at a speed of hypot(a, b).
    using Trait = omath::source_engine::PredEngineTrait;

    const auto basis = Trait::calc_view_basis(20.f, 35.f);
    const auto game_velocity = basis.forward * 1200.f + basis.up * 200.f;
    const auto offset_velocity = Trait::calc_view_basis(20.f + pipe_pitch_offset(), 35.f).forward * pipe_speed();

    EXPECT_NEAR(pipe_pitch_offset(), 9.4623f, 1e-3f);
    EXPECT_NEAR(pipe_speed(), 1216.55f, 1e-2f);
    EXPECT_NEAR(game_velocity.distance_to(offset_velocity), 0.f, 1e-2f);
}

TEST(UnitTestPredictionLauncher, PipeLauncherFiredLikeTheGameHits)
{
    using Trait = omath::source_engine::PredEngineTrait;
    constexpr float gravity = 800.f;
    constexpr Target target{.m_origin = {600, 150, 0}, .m_velocity = {-40, 30, 0}, .m_is_airborne = false};
    const Projectile proj = {.m_launch_speed = pipe_speed(), .m_gravity_scale = 0.5f};
    const Launcher launcher{
            .eye_origin = {0, 0, 64}, .muzzle_offset = k_source_muzzle, .launch_pitch_offset = pipe_pitch_offset()};

    const auto aim = Engine(gravity, 1.f / 1000.f, 5.f, 5.f).maybe_calculate_aim(proj, launcher, target);
    ASSERT_TRUE(aim.has_value());

    // Fire the way the game does rather than through the offset: forward * 1200 + up * 200 in the view frame, from the
    // view-relative muzzle, with the view angles the engine returned.
    const auto basis = Trait::calc_view_basis(aim->angles.pitch, aim->angles.yaw);
    const float time = aim->time_of_flight;
    auto position = launcher.launch_origin(basis) + (basis.forward * 1200.f + basis.up * 200.f) * time;
    position.z -= 0.5f * gravity * proj.m_gravity_scale * time * time;

    EXPECT_LE(position.distance_to(Trait::predict_target_position(target, time, gravity)), 6.f);

    // Left out, the same shot lands far away: the engine would aim the crosshair where the round has to go
    const Launcher no_offset{.eye_origin = {0, 0, 64}, .muzzle_offset = k_source_muzzle};
    const auto naive = Engine(gravity, 1.f / 1000.f, 5.f, 5.f).maybe_calculate_aim(proj, no_offset, target);
    ASSERT_TRUE(naive.has_value());
    const auto naive_basis = Trait::calc_view_basis(naive->angles.pitch, naive->angles.yaw);
    const float naive_time = naive->time_of_flight;
    auto naive_position =
            no_offset.launch_origin(naive_basis) + (naive_basis.forward * 1200.f + naive_basis.up * 200.f) * naive_time;
    naive_position.z -= 0.5f * gravity * proj.m_gravity_scale * naive_time * naive_time;
    EXPECT_GT(naive_position.distance_to(Trait::predict_target_position(target, naive_time, gravity)), 50.f);
}

TEST(UnitTestPredictionLauncher, PitchOffsetWithSideAndAirborneTargets)
{
    const Projectile proj = {.m_launch_speed = pipe_speed(), .m_gravity_scale = 0.5f};
    const Launcher launcher{
            .eye_origin = {0, 0, 64}, .muzzle_offset = k_source_muzzle, .launch_pitch_offset = pipe_pitch_offset()};

    constexpr Target side{.m_origin = {-50, 300, 80}, .m_velocity = {10, -5, 0}, .m_is_airborne = false};
    expect_launcher_solution_hits(proj, launcher, side, 800, 1.f / 1000.f, 30, 5.f, 10.f);

    constexpr Target airborne{.m_origin = {400, -100, 300}, .m_velocity = {10, -5, -20}, .m_is_airborne = true};
    expect_launcher_solution_hits(proj, launcher, airborne, 800, 1.f / 1000.f, 30, 5.f, 10.f);

    // A weapon that fires below the crosshair
    const Launcher downward{.eye_origin = {0, 0, 64}, .muzzle_offset = k_source_muzzle, .launch_pitch_offset = -4.f};
    expect_launcher_solution_hits(proj, downward, side, 800, 1.f / 1000.f, 30, 5.f, 10.f);
}

TEST(UnitTestPredictionLauncher, PitchOffsetShiftsViewPitchByExactlyTheOffset)
{
    // Without a rotating muzzle the launch is the same shot either way; only the view pitch that produces it moves
    constexpr Target target{.m_origin = {500, 100, 40}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const Projectile proj = {.m_launch_speed = pipe_speed(), .m_gravity_scale = 0.5f};
    const Engine engine(800.f, 1.f / 1000.f, 5.f, 5.f);

    const auto plain = engine.maybe_calculate_aim(proj, Launcher{.eye_origin = {0, 0, 64}}, target);
    const auto offset = engine.maybe_calculate_aim(
            proj, Launcher{.eye_origin = {0, 0, 64}, .launch_pitch_offset = pipe_pitch_offset()}, target);
    ASSERT_TRUE(plain.has_value());
    ASSERT_TRUE(offset.has_value());

    EXPECT_NEAR(offset->angles.pitch, plain->angles.pitch - pipe_pitch_offset(), 1e-4f);
    EXPECT_FLOAT_EQ(offset->angles.yaw, plain->angles.yaw);
    EXPECT_FLOAT_EQ(offset->time_of_flight, plain->time_of_flight);

    // The aim point follows the view, not the launch
    const auto [cam_pitch, cam_yaw, cam_roll] =
            omath::source_engine::CameraTrait::calc_look_at_angle({0, 0, 64}, offset->aim_point);
    EXPECT_NEAR(offset->angles.pitch, -cam_pitch.as_degrees(), 0.01f);
}

TEST(UnitTestPredictionLauncher, UnreachableViewPitchIsRejected)
{
    // Almost straight down: the launch pitch is about -88, so with a +9.46 offset the view would have to sit near
    // -97.5. Source stops at -89, so nobody can set that shot up.
    constexpr Target below{.m_origin = {30, 0, 0}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const Projectile proj = {.m_launch_speed = pipe_speed(), .m_gravity_scale = 0.5f};
    const Engine engine(800.f, 1.f / 1000.f, 5.f, 5.f);

    const auto plain = engine.maybe_calculate_aim(proj, Launcher{.eye_origin = {0, 0, 1000}}, below);
    ASSERT_TRUE(plain.has_value());
    EXPECT_LT(plain->angles.pitch, -85.f);

    EXPECT_FALSE(
            engine.maybe_calculate_aim(
                          proj, Launcher{.eye_origin = {0, 0, 1000}, .launch_pitch_offset = pipe_pitch_offset()}, below)
                    .has_value());
}
