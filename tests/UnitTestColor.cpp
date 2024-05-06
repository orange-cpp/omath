#include <gtest/gtest.h>
#include <uml/ProjectilePredictor.h>

TEST(x,x)
{
    uml::prediction::Target target{.m_origin = {100, 0, 60}, .m_vecVelocity = {0,0, 0}, .m_IsAirborne = false};
    uml::prediction::Projectile proj = {.m_origin = {3,2,1}, .m_velocity = 400, .m_gravityMultiplier= 0.4};
    auto vel = uml::prediction::ProjectilePredictor(400).PredictPointToAim(target, proj);

    auto pitch = proj.m_origin.ViewAngleTo(vel.value()).x;

    // printf("pitch: %f", pitch);
}