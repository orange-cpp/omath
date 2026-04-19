//
// Created by Vladislav on 20.04.2026.
//
#include <gtest/gtest.h>
#include <omath/engines/cry_engine/traits/pred_engine_trait.hpp>
#include <omath/projectile_prediction/projectile.hpp>
#include <omath/projectile_prediction/target.hpp>

using namespace omath;
using namespace omath::cry_engine;

// ---- predict_projectile_position ----

TEST(CryPredEngineTrait, PredictProjectilePositionAtTimeZero)
{
    projectile_prediction::Projectile p;
    p.m_origin       = {1.f, 2.f, 3.f};
    p.m_launch_offset = {4.f, 5.f, 6.f};
    p.m_launch_speed  = 100.f;
    p.m_gravity_scale = 1.f;

    const auto pos = PredEngineTrait::predict_projectile_position(p, 0.f, 0.f, 0.f, 9.81f);

    // At t=0 no velocity is applied, just origin+offset
    EXPECT_NEAR(pos.x, 5.f, 1e-4f);
    EXPECT_NEAR(pos.y, 7.f, 1e-4f);
    EXPECT_NEAR(pos.z, 9.f, 1e-4f);
}

TEST(CryPredEngineTrait, PredictProjectilePositionZeroAnglesForwardIsY)
{
    // Cry engine forward = +Y. At pitch=0, yaw=0 the projectile travels along +Y.
    projectile_prediction::Projectile p;
    p.m_origin       = {0.f, 0.f, 0.f};
    p.m_launch_speed  = 10.f;
    p.m_gravity_scale = 0.f; // no gravity so we isolate direction

    const auto pos = PredEngineTrait::predict_projectile_position(p, 0.f, 0.f, 1.f, 9.81f);

    EXPECT_NEAR(pos.x, 0.f, 1e-4f);
    EXPECT_NEAR(pos.y, 10.f, 1e-4f);
    EXPECT_NEAR(pos.z, 0.f, 1e-4f);
}

TEST(CryPredEngineTrait, PredictProjectilePositionGravityDropsZ)
{
    projectile_prediction::Projectile p;
    p.m_origin       = {0.f, 0.f, 0.f};
    p.m_launch_speed  = 10.f;
    p.m_gravity_scale = 1.f;

    const auto pos = PredEngineTrait::predict_projectile_position(p, 0.f, 0.f, 2.f, 9.81f);

    // z = 0 - (9.81 * 1) * (4) * 0.5 = -19.62
    EXPECT_NEAR(pos.z, -9.81f * 4.f * 0.5f, 1e-3f);
}

TEST(CryPredEngineTrait, PredictProjectilePositionGravityScaleZeroNoZDrop)
{
    projectile_prediction::Projectile p;
    p.m_origin       = {0.f, 0.f, 0.f};
    p.m_launch_speed  = 10.f;
    p.m_gravity_scale = 0.f;

    const auto pos = PredEngineTrait::predict_projectile_position(p, 0.f, 0.f, 3.f, 9.81f);

    EXPECT_NEAR(pos.z, 0.f, 1e-4f);
}

TEST(CryPredEngineTrait, PredictProjectilePositionWithLaunchOffset)
{
    projectile_prediction::Projectile p;
    p.m_origin        = {5.f, 0.f, 0.f};
    p.m_launch_offset = {0.f, 0.f, 2.f};
    p.m_launch_speed  = 10.f;
    p.m_gravity_scale = 0.f;

    const auto pos = PredEngineTrait::predict_projectile_position(p, 0.f, 0.f, 1.f, 0.f);

    // launch position = {5, 0, 2}, travels along +Y by 10
    EXPECT_NEAR(pos.x, 5.f,  1e-4f);
    EXPECT_NEAR(pos.y, 10.f, 1e-4f);
    EXPECT_NEAR(pos.z, 2.f,  1e-4f);
}

// ---- predict_target_position ----

TEST(CryPredEngineTrait, PredictTargetPositionGroundedStationary)
{
    projectile_prediction::Target t;
    t.m_origin     = {10.f, 20.f, 5.f};
    t.m_velocity   = {0.f, 0.f, 0.f};
    t.m_is_airborne = false;

    const auto pred = PredEngineTrait::predict_target_position(t, 5.f, 9.81f);

    EXPECT_NEAR(pred.x, 10.f, 1e-6f);
    EXPECT_NEAR(pred.y, 20.f, 1e-6f);
    EXPECT_NEAR(pred.z,  5.f, 1e-6f);
}

TEST(CryPredEngineTrait, PredictTargetPositionGroundedMoving)
{
    projectile_prediction::Target t;
    t.m_origin     = {0.f, 0.f, 0.f};
    t.m_velocity   = {3.f, 4.f, 0.f};
    t.m_is_airborne = false;

    const auto pred = PredEngineTrait::predict_target_position(t, 2.f, 9.81f);

    EXPECT_NEAR(pred.x, 6.f,  1e-6f);
    EXPECT_NEAR(pred.y, 8.f,  1e-6f);
    EXPECT_NEAR(pred.z, 0.f,  1e-6f); // grounded — no gravity
}

TEST(CryPredEngineTrait, PredictTargetPositionAirborneGravityDropsZ)
{
    projectile_prediction::Target t;
    t.m_origin     = {0.f, 0.f, 20.f};
    t.m_velocity   = {0.f, 0.f, 0.f};
    t.m_is_airborne = true;

    const auto pred = PredEngineTrait::predict_target_position(t, 2.f, 9.81f);

    // z = 20 - 9.81 * 4 * 0.5 = 20 - 19.62 = 0.38
    EXPECT_NEAR(pred.z, 20.f - 9.81f * 4.f * 0.5f, 1e-4f);
}

TEST(CryPredEngineTrait, PredictTargetPositionAirborneMovingWithGravity)
{
    projectile_prediction::Target t;
    t.m_origin     = {0.f, 0.f, 50.f};
    t.m_velocity   = {10.f, 5.f, 0.f};
    t.m_is_airborne = true;

    const auto pred = PredEngineTrait::predict_target_position(t, 3.f, 9.81f);

    EXPECT_NEAR(pred.x, 30.f, 1e-4f);
    EXPECT_NEAR(pred.y, 15.f, 1e-4f);
    EXPECT_NEAR(pred.z, 50.f - 9.81f * 9.f * 0.5f, 1e-4f);
}

// ---- calc_vector_2d_distance ----

TEST(CryPredEngineTrait, CalcVector2dDistance_3_4_5)
{
    EXPECT_NEAR(PredEngineTrait::calc_vector_2d_distance({3.f, 4.f, 999.f}), 5.f, 1e-5f);
}

TEST(CryPredEngineTrait, CalcVector2dDistance_ZeroVector)
{
    EXPECT_NEAR(PredEngineTrait::calc_vector_2d_distance({0.f, 0.f, 0.f}), 0.f, 1e-6f);
}

TEST(CryPredEngineTrait, CalcVector2dDistance_ZIgnored)
{
    // Z does not affect the 2D distance
    EXPECT_NEAR(PredEngineTrait::calc_vector_2d_distance({0.f, 5.f, 100.f}),
                PredEngineTrait::calc_vector_2d_distance({0.f, 5.f, 0.f}), 1e-6f);
}

// ---- get_vector_height_coordinate ----

TEST(CryPredEngineTrait, GetVectorHeightCoordinate_ReturnsZ)
{
    // Cry engine up = +Z
    EXPECT_FLOAT_EQ(PredEngineTrait::get_vector_height_coordinate({1.f, 2.f, 7.f}), 7.f);
}

// ---- calc_direct_pitch_angle ----

TEST(CryPredEngineTrait, CalcDirectPitchAngle_Flat)
{
    // Target at same height → pitch = 0
    EXPECT_NEAR(PredEngineTrait::calc_direct_pitch_angle({0.f, 0.f, 0.f}, {0.f, 100.f, 0.f}), 0.f, 1e-4f);
}

TEST(CryPredEngineTrait, CalcDirectPitchAngle_LookingUp)
{
    // Target at 45° above (equal XY distance and Z height)
    // direction to {0, 1, 1} normalized = {0, 0.707, 0.707}, asin(0.707) = 45°
    EXPECT_NEAR(PredEngineTrait::calc_direct_pitch_angle({0.f, 0.f, 0.f}, {0.f, 1.f, 1.f}), 45.f, 1e-3f);
}

TEST(CryPredEngineTrait, CalcDirectPitchAngle_LookingDown)
{
    // Target directly below
    EXPECT_NEAR(PredEngineTrait::calc_direct_pitch_angle({0.f, 0.f, 10.f}, {0.f, 0.f, 0.f}), -90.f, 1e-3f);
}

TEST(CryPredEngineTrait, CalcDirectPitchAngle_LookingDirectlyUp)
{
    EXPECT_NEAR(PredEngineTrait::calc_direct_pitch_angle({0.f, 0.f, 0.f}, {0.f, 0.f, 100.f}), 90.f, 1e-3f);
}

// ---- calc_direct_yaw_angle ----

TEST(CryPredEngineTrait, CalcDirectYawAngle_ForwardAlongY)
{
    // Cry engine forward = +Y → yaw = 0
    EXPECT_NEAR(PredEngineTrait::calc_direct_yaw_angle({0.f, 0.f, 0.f}, {0.f, 100.f, 0.f}), 0.f, 1e-4f);
}

TEST(CryPredEngineTrait, CalcDirectYawAngle_AlongPositiveX)
{
    // direction = {1, 0, 0}, yaw = -atan2(1, 0) = -90°
    EXPECT_NEAR(PredEngineTrait::calc_direct_yaw_angle({0.f, 0.f, 0.f}, {100.f, 0.f, 0.f}), -90.f, 1e-3f);
}

TEST(CryPredEngineTrait, CalcDirectYawAngle_AlongNegativeX)
{
    // direction = {-1, 0, 0}, yaw = -atan2(-1, 0) = 90°
    EXPECT_NEAR(PredEngineTrait::calc_direct_yaw_angle({0.f, 0.f, 0.f}, {-100.f, 0.f, 0.f}), 90.f, 1e-3f);
}

TEST(CryPredEngineTrait, CalcDirectYawAngle_BackwardAlongNegY)
{
    // direction = {0, -1, 0}, yaw = -atan2(0, -1) = ±180°
    const float yaw = PredEngineTrait::calc_direct_yaw_angle({0.f, 0.f, 0.f}, {0.f, -100.f, 0.f});
    EXPECT_NEAR(std::abs(yaw), 180.f, 1e-3f);
}

TEST(CryPredEngineTrait, CalcDirectYawAngle_OffOriginCamera)
{
    // Same relative direction regardless of camera position
    const float yaw_a = PredEngineTrait::calc_direct_yaw_angle({0.f, 0.f, 0.f}, {0.f, 100.f, 0.f});
    const float yaw_b = PredEngineTrait::calc_direct_yaw_angle({50.f, 50.f, 0.f}, {50.f, 150.f, 0.f});
    EXPECT_NEAR(yaw_a, yaw_b, 1e-4f);
}

// ---- calc_viewpoint_from_angles ----

TEST(CryPredEngineTrait, CalcViewpointFromAngles_45Degrees)
{
    projectile_prediction::Projectile p;
    p.m_origin      = {0.f, 0.f, 0.f};
    p.m_launch_speed = 10.f;

    // Target along +Y at distance 10; pitch=45° → height = 10 * tan(45°) = 10
    const Vector3<float> target{0.f, 10.f, 0.f};
    const auto vp = PredEngineTrait::calc_viewpoint_from_angles(p, target, 45.f);

    EXPECT_NEAR(vp.x, 0.f,  1e-4f);
    EXPECT_NEAR(vp.y, 10.f, 1e-4f);
    EXPECT_NEAR(vp.z, 10.f, 1e-3f);
}

TEST(CryPredEngineTrait, CalcViewpointFromAngles_ZeroPitch)
{
    projectile_prediction::Projectile p;
    p.m_origin      = {0.f, 0.f, 5.f};
    p.m_launch_speed = 1.f;

    const Vector3<float> target{3.f, 4.f, 0.f};
    const auto vp = PredEngineTrait::calc_viewpoint_from_angles(p, target, 0.f);

    // tan(0) = 0 → viewpoint Z = origin.z + 0 = 5
    EXPECT_NEAR(vp.x, 3.f, 1e-4f);
    EXPECT_NEAR(vp.y, 4.f, 1e-4f);
    EXPECT_NEAR(vp.z, 5.f, 1e-4f);
}

TEST(CryPredEngineTrait, CalcViewpointXYMatchesPredictedTargetXY)
{
    projectile_prediction::Projectile p;
    p.m_origin      = {1.f, 2.f, 3.f};
    p.m_launch_speed = 50.f;

    const Vector3<float> target{10.f, 20.f, 5.f};
    const auto vp = PredEngineTrait::calc_viewpoint_from_angles(p, target, 30.f);

    // X and Y always match the predicted target position
    EXPECT_NEAR(vp.x, target.x, 1e-4f);
    EXPECT_NEAR(vp.y, target.y, 1e-4f);
}
