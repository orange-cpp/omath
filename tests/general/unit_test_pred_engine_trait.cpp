// Tests for PredEngineTrait
#include <gtest/gtest.h>
#include <omath/engines/source_engine/traits/pred_engine_trait.hpp>
#include <omath/projectile_prediction/projectile.hpp>
#include <omath/projectile_prediction/target.hpp>

using namespace omath;
using namespace omath::source_engine;

TEST(PredEngineTrait, PredictProjectilePositionBasic)
{
    projectile_prediction::Projectile p;
    p.m_origin = {0.f, 0.f, 0.f};
    p.m_launch_speed = 10.f;
    p.m_gravity_scale = 1.f;

    const auto pos = PredEngineTrait::predict_projectile_position(p, /*pitch*/ 0.f, /*yaw*/ 0.f, /*time*/ 1.f,
                                                                  /*gravity*/ 9.81f);
    // With zero pitch and yaw forward vector is along X; expect x ~10, z reduced by gravity*0.5
    EXPECT_NEAR(pos.x, 10.f, 1e-3f);
    EXPECT_NEAR(pos.z, -9.81f * 0.5f, 1e-3f);
}

TEST(PredEngineTrait, PredictTargetPositionAirborne)
{
    projectile_prediction::Target t;
    t.m_origin = {0.f, 0.f, 10.f};
    t.m_velocity = {1.f, 0.f, 0.f};
    t.m_is_airborne = true;

    const auto pred = PredEngineTrait::predict_target_position(t, 2.f, 9.81f);
    EXPECT_NEAR(pred.x, 2.f, 1e-6f);
    // z should have been reduced by gravity* t^2
    EXPECT_NEAR(pred.z, 10.f - 9.81f * 4.f * 0.5f, 1e-6f);
}

TEST(PredEngineTrait, CalcVector2dDistance)
{
    constexpr Vector3<float> d{3.f, 4.f, 0.f};
    EXPECT_NEAR(PredEngineTrait::calc_vector_2d_distance(d), 5.f, 1e-6f);
}

TEST(PredEngineTrait, CalcViewpointFromAngles)
{
    projectile_prediction::Projectile p;
    p.m_origin = {0.f, 0.f, 0.f};
    p.m_launch_speed = 10.f;

    constexpr Vector3<float> predicted{10.f, 0.f, 0.f};
    constexpr std::optional<float> pitch = 45.f;
    const auto vp = PredEngineTrait::calc_viewpoint_from_angles(p, predicted, pitch);
    // For 45 degrees, height = delta2d * tan(45deg) = 10 * 1 = 10
    EXPECT_NEAR(vp.z, 10.f, 1e-6f);
}

TEST(PredEngineTrait, DirectAngles)
{
    constexpr Vector3<float> origin{0.f, 0.f, 0.f};
    constexpr Vector3<float> target{0.f, 1.f, 1.f};
    // yaw should be 90 degrees (pointing along y)
    EXPECT_NEAR(PredEngineTrait::calc_direct_yaw_angle(origin, target), 90.f, 1e-3f);
    // pitch should be asin(z/distance)
    const float dist = origin.distance_to(target);
    EXPECT_NEAR(PredEngineTrait::calc_direct_pitch_angle(origin, target),
                angles::radians_to_degrees(std::asin((target.z - origin.z) / dist)), 1e-3f);
}
