#include <gtest/gtest.h>
#include <limits>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>
#include <omath/projectile_prediction/projectile.hpp>
#include <omath/projectile_prediction/target.hpp>
#include <omath/linear_algebra/vector3.hpp>

using Projectile = omath::projectile_prediction::Projectile<float>;
using Target     = omath::projectile_prediction::Target<float>;
using omath::Vector3;

// Fake engine trait where gravity is effectively zero and projectile prediction always hits the target
struct FakeEngineZeroGravity
{
    static Vector3<float> predict_target_position(const Target& t, float /*time*/, float /*gravity*/) noexcept
    {
        return t.m_origin;
    }
    static Vector3<float> predict_projectile_position(const Projectile& /*p*/, float /*pitch*/, float /*yaw*/,
                                                      float /*time*/, float /*gravity*/) noexcept
    {
        // Return a fixed point matching typical target used in the test
        return Vector3<float>{100.f, 0.f, 0.f};
    }
    static float calc_vector_2d_distance(const Vector3<float>& v) noexcept
    {
        return std::hypot(v.x, v.y);
    }
    static float get_vector_height_coordinate(const Vector3<float>& v) noexcept
    {
        return v.z;
    }
    static Vector3<float> calc_viewpoint_from_angles(const Projectile& /*p*/, Vector3<float> /*v*/,
                                                     std::optional<float> /*maybe_pitch*/) noexcept
    {
        return Vector3<float>{1.f, 2.f, 3.f};
    }
    static float calc_direct_pitch_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept
    {
        return 12.5f;
    }
    static float calc_direct_yaw_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept
    {
        return 0.f;
    }
    static bool can_projectile_reach_target_at_time(const Projectile& /*projectile*/,
                                                    const Vector3<float>& /*target_position*/, float /*time*/,
                                                    float /*gravity*/, float /*distance_tolerance*/)
    {
        return false;
    }
};

TEST(ProjPredLegacyMore, ZeroGravityUsesDirectPitchAndReturnsViewpoint)
{
    constexpr Projectile proj{.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 10.f, .m_gravity_scale = 0.f};
    constexpr Target target{.m_origin = {100.f, 0.f, 0.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false};

    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<FakeEngineZeroGravity>;
    const Engine engine(9.8f, 0.1f, 5.f, 1e-3f);

    const auto res = engine.maybe_calculate_aim_point(proj, target);
    ASSERT_TRUE(res.has_value());
    const auto v = res.value();
    EXPECT_NEAR(v.x, 1.f, 1e-6f);
    EXPECT_NEAR(v.y, 2.f, 1e-6f);
    EXPECT_NEAR(v.z, 3.f, 1e-6f);
}

TEST(ProjPredLegacyMore, ZeroGravityAimAnglesReturnsPitchAndYaw)
{
    constexpr Projectile proj{ .m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 10.f, .m_gravity_scale = 0.f };
    constexpr Target target{ .m_origin = {100.f, 0.f, 0.f}, .m_velocity = {0.f,0.f,0.f}, .m_is_airborne = false };

    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<FakeEngineZeroGravity>;
    const Engine engine(9.8f, 0.1f, 5.f, 1e-3f);

    const auto res = engine.maybe_calculate_aim_angles(proj, target);
    ASSERT_TRUE(res.has_value());
    // FakeEngineZeroGravity::calc_direct_pitch_angle returns 12.5f
    EXPECT_NEAR(res->pitch, 12.5f, 1e-6f);
    // FakeEngineZeroGravity::calc_direct_yaw_angle returns 0.f
    EXPECT_NEAR(res->yaw, 0.f, 1e-6f);
}

// Fake trait producing no valid launch angle (root < 0)
struct FakeEngineNoSolution
{
    static Vector3<float> predict_target_position(const Target& t, float /*time*/, float /*gravity*/) noexcept { return t.m_origin; }
    static Vector3<float> predict_projectile_position(const Projectile& /*p*/, float /*pitch*/, float /*yaw*/, float /*time*/, float /*gravity*/) noexcept { return Vector3<float>{0.f,0.f,0.f}; }
    static float calc_vector_2d_distance(const Vector3<float>& /*v*/) noexcept { return 10000.f; }
    static float get_vector_height_coordinate(const Vector3<float>& /*v*/) noexcept { return 0.f; }
    static Vector3<float> calc_viewpoint_from_angles(const Projectile& /*p*/, Vector3<float> /*v*/, std::optional<float> /*maybe_pitch*/) noexcept { return Vector3<float>{}; }
    static float calc_direct_pitch_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept { return 0.f; }
    static float calc_direct_yaw_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept { return 0.f; }
};

TEST(ProjPredLegacyMore, NoSolutionRootReturnsNullopt)
{
    // Very slow projectile and large distance -> quadratic root negative
    constexpr Projectile proj{ .m_origin = {0.f,0.f,0.f}, .m_launch_speed = 1.f, .m_gravity_scale = 1.f };
    constexpr Target target{ .m_origin = {10000.f, 0.f, 0.f}, .m_velocity = {0.f,0.f,0.f}, .m_is_airborne = false };

    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<FakeEngineNoSolution>;
    const Engine engine(9.8f, 0.5f, 2.f, 1.f);

    const auto res = engine.maybe_calculate_aim_point(proj, target);
    EXPECT_FALSE(res.has_value());

    const auto angles_res = engine.maybe_calculate_aim_angles(proj, target);
    EXPECT_FALSE(angles_res.has_value());
}

// Fake trait where an angle exists but the projectile does not reach target (miss)
struct FakeEngineAngleButMiss
{
    static Vector3<float> predict_target_position(const Target& t, float /*time*/, float /*gravity*/) noexcept { return t.m_origin; }
    static Vector3<float> predict_projectile_position(const Projectile& /*p*/, float /*pitch*/, float /*yaw*/, float /*time*/, float /*gravity*/) noexcept
    {
        // always return a point far from the target
        return Vector3<float>{0.f, 0.f, 1000.f};
    }
    static float calc_vector_2d_distance(const Vector3<float>& v) noexcept { return std::hypot(v.x, v.y); }
    static float get_vector_height_coordinate(const Vector3<float>& v) noexcept { return v.z; }
    static Vector3<float> calc_viewpoint_from_angles(const Projectile& /*p*/, Vector3<float> /*v*/, std::optional<float> /*maybe_pitch*/) noexcept { return Vector3<float>{9.f,9.f,9.f}; }
    static float calc_direct_pitch_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept { return 1.f; }
    static float calc_direct_yaw_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept { return 0.f; }
};

TEST(ProjPredLegacyMore, AngleComputedButMissReturnsNullopt)
{
    constexpr Projectile proj{ .m_origin = {0.f,0.f,0.f}, .m_launch_speed = 100.f, .m_gravity_scale = 1.f };
    constexpr Target target{ .m_origin = {10.f, 0.f, 0.f}, .m_velocity = {0.f,0.f,0.f}, .m_is_airborne = false };

    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<FakeEngineAngleButMiss>;
    const Engine engine(9.8f, 0.1f, 1.f, 0.1f);

    const auto res = engine.maybe_calculate_aim_point(proj, target);
    EXPECT_FALSE(res.has_value());
}

TEST(ProjPredLegacyMore, IncludesMaximumSimulationTime)
{
    constexpr Projectile projectile{.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 10.f, .m_gravity_scale = 0.f};
    constexpr Target target{.m_origin = {10.f, 0.f, 0.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false};
    const omath::projectile_prediction::ProjPredEngineLegacy<> engine(0.f, 0.1f, 1.f, 0.f);

    const auto result = engine.maybe_calculate_aim_point(projectile, target);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, target.m_origin.x, 1e-6f);
    EXPECT_NEAR(result->y, target.m_origin.y, 1e-6f);
    EXPECT_NEAR(result->z, target.m_origin.z, 1e-6f);
}

TEST(ProjPredLegacyMore, RejectsInvalidSimulationSteps)
{
    constexpr Projectile projectile{.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 10.f, .m_gravity_scale = 0.f};
    constexpr Target target{.m_origin = {10.f, 0.f, 0.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false};

    EXPECT_FALSE(omath::projectile_prediction::ProjPredEngineLegacy<>(0.f, 0.f, 1.f, 1.f)
                         .maybe_calculate_aim_point(projectile, target));
    EXPECT_FALSE(omath::projectile_prediction::ProjPredEngineLegacy<>(0.f, -0.1f, 1.f, 1.f)
                         .maybe_calculate_aim_point(projectile, target));
    EXPECT_FALSE(
            omath::projectile_prediction::ProjPredEngineLegacy<>(0.f, std::numeric_limits<float>::infinity(), 1.f, 1.f)
                    .maybe_calculate_aim_point(projectile, target));
}

TEST(ProjPredLegacyMore, RejectsInvalidProjectileSpeedAndTolerance)
{
    constexpr Target target{.m_origin = {1.f, 0.f, 0.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false};
    constexpr Projectile stopped_projectile{.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 0.f, .m_gravity_scale = 0.f};
    constexpr Projectile moving_projectile{.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 10.f, .m_gravity_scale = 0.f};

    EXPECT_FALSE(omath::projectile_prediction::ProjPredEngineLegacy<>(0.f, 0.1f, 1.f, 2.f)
                         .maybe_calculate_aim_point(stopped_projectile, target));
    EXPECT_FALSE(omath::projectile_prediction::ProjPredEngineLegacy<>(0.f, 0.1f, 1.f, -1.f)
                         .maybe_calculate_aim_point(moving_projectile, target));
}

TEST(ProjPredLegacyMore, StablePitchFindsHighSpeedLowArc)
{
    constexpr Projectile projectile{.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 10'000.f, .m_gravity_scale = 1.f};
    constexpr Target target{.m_origin = {100.f, 0.f, 0.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false};
    const omath::projectile_prediction::ProjPredEngineLegacy<> engine(9.81f, 0.01f, 0.02f, 0.0001f);

    const auto result = engine.maybe_calculate_aim_angles(projectile, target);

    ASSERT_TRUE(result.has_value());
    EXPECT_GT(result->pitch, 0.f);
    EXPECT_NEAR(result->yaw, 0.f, 1e-6f);
}

TEST(ProjPredLegacyMore, CoincidentTargetReturnsLaunchOrigin)
{
    constexpr Projectile projectile{.m_origin = {5.f, 4.f, 3.f}, .m_launch_speed = 100.f, .m_gravity_scale = 1.f};
    constexpr Target target{.m_origin = projectile.m_origin, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false};
    const omath::projectile_prediction::ProjPredEngineLegacy<> engine(9.81f, 0.01f, 0.01f, 0.f);

    const auto result = engine.maybe_calculate_aim_point(projectile, target);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), projectile.m_origin);
}

TEST(ProjPredLegacyMore, TinyDistanceDoesNotUnderflowToAHit)
{
    constexpr Projectile projectile{.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 1.f, .m_gravity_scale = 0.f};
    constexpr Target target{.m_origin = {1e-30f, 0.f, 0.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false};
    const omath::projectile_prediction::ProjPredEngineLegacy<> engine(0.f, 0.1f, 0.f, 0.f);

    EXPECT_FALSE(engine.maybe_calculate_aim_point(projectile, target));
}
