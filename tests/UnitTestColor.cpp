#include <gtest/gtest.h>
#include <uml/prediction/Engine.h>

TEST(x,x)
{
    uml::prediction::Target target{.m_origin = {100, 0, 60}, .m_velocity = {0, 0, 0}, .m_isAirborne = false};
    uml::prediction::Projectile proj = {.m_origin = {3,2,1}, .m_launchSpeed = 400, .m_gravityScale= 0.4};
    auto vel = uml::prediction::Engine(400, 1.f / 10000.f, 50).MaybeCalculateAimPoint(proj, target);

    auto pitch = proj.m_origin.ViewAngleTo(vel.value()).x;

    printf("pitch: %f", pitch);
}