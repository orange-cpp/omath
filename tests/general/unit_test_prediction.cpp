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
