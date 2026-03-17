#include <gtest/gtest.h>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>
#include <omath/engines/source_engine/traits/camera_trait.hpp>
TEST(UnitTestPrediction, PredictionTest)
{
    constexpr omath::projectile_prediction::Target target{
            .m_origin = {100, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr omath::projectile_prediction::Projectile proj = {
            .m_origin = {3, 2, 1}, .m_launch_speed = 5000, .m_gravity_scale = 0.4};
    const auto viewPoint =
            omath::projectile_prediction::ProjPredEngineLegacy(400, 1.f / 1000.f, 50, 5.f).maybe_calculate_aim_point(proj, target);


    const auto [pitch, yaw, _] =omath::source_engine::CameraTrait::calc_look_at_angle(proj.m_origin, viewPoint.value());

    EXPECT_NEAR(-42.547142, pitch.as_degrees(), 0.01f);
    EXPECT_NEAR(-1.181189, yaw.as_degrees(), 0.01f);
}

// Helper: verify aim_angles match angles derived from aim_point via CameraTrait
static void expect_angles_match_aim_point(const omath::projectile_prediction::Projectile& proj,
                                          const omath::projectile_prediction::Target& target,
                                          float gravity, float step, float max_time, float tolerance,
                                          float angle_eps = 0.01f)
{
    const omath::projectile_prediction::ProjPredEngineLegacy engine(gravity, step, max_time, tolerance);

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
    constexpr omath::projectile_prediction::Target target{
            .m_origin = {100, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr omath::projectile_prediction::Projectile proj = {
            .m_origin = {3, 2, 1}, .m_launch_speed = 5000, .m_gravity_scale = 0.4};

    expect_angles_match_aim_point(proj, target, 400, 1.f / 1000.f, 50, 5.f);
}

TEST(UnitTestPrediction, AimAnglesMatchAimPoint_MovingTarget)
{
    constexpr omath::projectile_prediction::Target target{
            .m_origin = {500, 100, 0}, .m_velocity = {-50, 20, 0}, .m_is_airborne = false};
    constexpr omath::projectile_prediction::Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_speed = 3000, .m_gravity_scale = 1.0};

    expect_angles_match_aim_point(proj, target, 800, 1.f / 500.f, 30, 10.f);
}

TEST(UnitTestPrediction, AimAnglesMatchAimPoint_AirborneTarget)
{
    constexpr omath::projectile_prediction::Target target{
            .m_origin = {200, 50, 300}, .m_velocity = {10, -5, -20}, .m_is_airborne = true};
    constexpr omath::projectile_prediction::Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_speed = 4000, .m_gravity_scale = 0.5};

    expect_angles_match_aim_point(proj, target, 400, 1.f / 1000.f, 50, 10.f);
}

TEST(UnitTestPrediction, AimAnglesMatchAimPoint_HighArc)
{
    // Target nearly directly above — high pitch angle
    constexpr omath::projectile_prediction::Target target{
            .m_origin = {10, 0, 500}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr omath::projectile_prediction::Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_speed = 5000, .m_gravity_scale = 0.3};

    expect_angles_match_aim_point(proj, target, 400, 1.f / 1000.f, 50, 5.f);
}

TEST(UnitTestPrediction, AimAnglesMatchAimPoint_NegativeYaw)
{
    // Target behind and to the left — negative yaw quadrant
    constexpr omath::projectile_prediction::Target target{
            .m_origin = {-200, -150, 10}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr omath::projectile_prediction::Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_speed = 5000, .m_gravity_scale = 0.4};

    expect_angles_match_aim_point(proj, target, 400, 1.f / 1000.f, 50, 5.f);
}

TEST(UnitTestPrediction, AimAnglesMatchAimPoint_WithLaunchOffset)
{
    constexpr omath::projectile_prediction::Target target{
            .m_origin = {200, 0, 50}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    const omath::projectile_prediction::Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_offset = {5, 0, -3}, .m_launch_speed = 5000, .m_gravity_scale = 0.4};

    expect_angles_match_aim_point(proj, target, 400, 1.f / 1000.f, 50, 5.f);
}

TEST(UnitTestPrediction, AimAnglesReturnsNulloptWhenNoSolution)
{
    constexpr omath::projectile_prediction::Target target{
            .m_origin = {100000, 0, 0}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr omath::projectile_prediction::Projectile proj = {
            .m_origin = {0, 0, 0}, .m_launch_speed = 1, .m_gravity_scale = 1};

    const omath::projectile_prediction::ProjPredEngineLegacy engine(9.81f, 0.1f, 2.f, 5.f);

    const auto aim_point = engine.maybe_calculate_aim_point(proj, target);
    const auto aim_angles = engine.maybe_calculate_aim_angles(proj, target);

    EXPECT_FALSE(aim_point.has_value());
    EXPECT_FALSE(aim_angles.has_value());
}
