#include <gtest/gtest.h>
#include <uml/ProjectilePredictor.h>
#include <print>


TEST(x,x)
{
    uml::prediction::Target target{.m_origin = {100, 0, 0}, .m_vecVelocity = {0,0, 0}, .m_IsAirborne = false};
    uml::prediction::Projectile proj = {.m_velocity = 600, .m_gravityMultiplier= 0.1};
    auto vel = uml::prediction::ProjectilePredictor(400).PredictPointToAim(target, proj);

    std::print("{}", vel.has_value());
}