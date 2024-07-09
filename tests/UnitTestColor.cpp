#include <gtest/gtest.h>
#include <omath/prediction/Engine.h>

TEST(x,x)
{
    omath::prediction::Target target{.m_origin = {100, 0, 60}, .m_velocity = {0, 0, 0}, .m_isAirborne = false};
    omath::prediction::Projectile proj = {.m_origin = {3,2,1}, .m_launchSpeed = 5000, .m_gravityScale= 0.4};
    auto vel = omath::prediction::Engine(400, 1.f / 1000.f, 50, 5.f).MaybeCalculateAimPoint(proj, target);

    auto pitch = proj.m_origin.ViewAngleTo(vel.value()).x;

    printf("pitch: %f", pitch);
}