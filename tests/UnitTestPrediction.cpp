#include <gtest/gtest.h>
#include <omath/prediction/Engine.hpp>

TEST(UnitTestPrediction, PredictionTest)
{
    const omath::prediction::Target target{.m_origin = {100, 0, 90}, .m_velocity = {0, 0, 0}, .m_isAirborne = false};
    const omath::prediction::Projectile proj = {.m_origin = {3,2,1}, .m_launchSpeed = 5000, .m_gravityScale= 0.4};
    const auto viewPoint = omath::prediction::Engine(400, 1.f / 1000.f, 50, 5.f).MaybeCalculateAimPoint(proj, target);

    const auto [pitch, yaw, _] = proj.m_origin.ViewAngleTo(viewPoint.value()).AsTuple();

    EXPECT_NEAR(42.547142, pitch, 0.0001f);
    EXPECT_NEAR(-1.181189, yaw, 0.0001f);
}