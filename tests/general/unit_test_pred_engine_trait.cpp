// Tests for PredEngineTrait
#include <gtest/gtest.h>
#include <omath/engines/source_engine/traits/pred_engine_trait.hpp>
#include <omath/projectile_prediction/projectile.hpp>
#include <omath/projectile_prediction/target.hpp>

using namespace omath;
using namespace omath::source_engine;

using Projectile = projectile_prediction::Projectile<float>;
using Target = projectile_prediction::Target<float>;

TEST(PredEngineTrait, PredictProjectilePositionBasic)
{
    Projectile p;
    p.m_origin = {0.f, 0.f, 0.f};
    p.m_launch_speed = 10.f;
    p.m_gravity_scale = 1.f;

    const auto pos = PredEngineTrait::predict_projectile_position(p.m_origin + p.m_launch_offset, p, /*pitch*/ 0.f,
                                                                  /*yaw*/ 0.f, /*time*/ 1.f,
                                                                  /*gravity*/ 9.81f);
    // With zero pitch and yaw forward vector is along X; expect x ~10, z reduced by gravity*0.5
    EXPECT_NEAR(pos.x, 10.f, 1e-3f);
    EXPECT_NEAR(pos.z, -9.81f * 0.5f, 1e-3f);
}

TEST(PredEngineTrait, PredictTargetPositionAirborne)
{
    Target t;
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

TEST(PredEngineTrait, CalcViewBasis)
{
    // Zero angles: Source looks down +X with +Z up
    const auto level = PredEngineTrait::calc_view_basis(0.f, 0.f);
    EXPECT_NEAR(level.forward.x, 1.f, 1e-5f);
    EXPECT_NEAR(level.forward.z, 0.f, 1e-5f);
    EXPECT_NEAR(level.up.z, 1.f, 1e-5f);

    // Trait pitch is positive upwards even though the engine's own pitch is negative upwards
    const auto raised = PredEngineTrait::calc_view_basis(45.f, 0.f);
    EXPECT_NEAR(raised.forward.x, std::cos(angles::degrees_to_radians(45.f)), 1e-5f);
    EXPECT_NEAR(raised.forward.z, std::sin(angles::degrees_to_radians(45.f)), 1e-5f);
    EXPECT_NEAR(raised.forward.dot(raised.right), 0.f, 1e-5f);
    EXPECT_NEAR(raised.forward.dot(raised.up), 0.f, 1e-5f);

    // Yaw swings forward towards +Y
    const auto turned = PredEngineTrait::calc_view_basis(0.f, 90.f);
    EXPECT_NEAR(turned.forward.y, 1.f, 1e-5f);
}

TEST(PredEngineTrait, PredictProjectilePositionWithLaunchOffset)
{
    Projectile p;
    p.m_origin = {0.f, 0.f, 0.f};
    p.m_launch_offset = {5.f, 3.f, -2.f};
    p.m_launch_speed = 10.f;
    p.m_gravity_scale = 1.f;

    // At time=0, projectile should be at launch_pos = origin + offset
    const auto pos_t0 =
            PredEngineTrait::predict_projectile_position(p.m_origin + p.m_launch_offset, p, 0.f, 0.f, 0.f, 9.81f);
    EXPECT_NEAR(pos_t0.x, 5.f, 1e-4f);
    EXPECT_NEAR(pos_t0.y, 3.f, 1e-4f);
    EXPECT_NEAR(pos_t0.z, -2.f, 1e-4f);

    // At time=1 with zero pitch/yaw, should travel along X from the offset position
    const auto pos_t1 =
            PredEngineTrait::predict_projectile_position(p.m_origin + p.m_launch_offset, p, 0.f, 0.f, 1.f, 9.81f);
    EXPECT_NEAR(pos_t1.x, 5.f + 10.f, 1e-3f);
    EXPECT_NEAR(pos_t1.y, 3.f, 1e-3f);
    EXPECT_NEAR(pos_t1.z, -2.f - 9.81f * 0.5f, 1e-3f);
}

TEST(PredEngineTrait, ZeroLaunchOffsetMatchesOriginalBehavior)
{
    Projectile p;
    p.m_origin = {10.f, 20.f, 30.f};
    p.m_launch_offset = {0.f, 0.f, 0.f};
    p.m_launch_speed = 15.f;
    p.m_gravity_scale = 0.5f;

    Projectile p_no_offset;
    p_no_offset.m_origin = {10.f, 20.f, 30.f};
    p_no_offset.m_launch_speed = 15.f;
    p_no_offset.m_gravity_scale = 0.5f;

    const auto pos1 =
            PredEngineTrait::predict_projectile_position(p.m_origin + p.m_launch_offset, p, 30.f, 45.f, 2.f, 9.81f);
    const auto pos2 = PredEngineTrait::predict_projectile_position(p_no_offset.m_origin + p_no_offset.m_launch_offset,
                                                                   p_no_offset, 30.f, 45.f, 2.f, 9.81f);
    EXPECT_NEAR(pos1.x, pos2.x, 1e-6f);
    EXPECT_NEAR(pos1.y, pos2.y, 1e-6f);
    EXPECT_NEAR(pos1.z, pos2.z, 1e-6f);
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
