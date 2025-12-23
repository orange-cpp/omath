#include <gtest/gtest.h>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>
#include <omath/projectile_prediction/projectile.hpp>
#include <omath/projectile_prediction/target.hpp>
#include <omath/linear_algebra/vector3.hpp>

using omath::projectile_prediction::Projectile;
using omath::projectile_prediction::Target;
using omath::Vector3;

// Fake engine trait where gravity is effectively zero and projectile prediction always hits the target
struct FakeEngineZeroGravity
{
    static Vector3<float> predict_target_position(const Target& t, float /*time*/, float /*gravity*/) noexcept
    {
        return t.m_origin;
    }
    static Vector3<float> predict_projectile_position(const Projectile& /*p*/, float /*pitch*/, float /*yaw*/, float /*time*/, float /*gravity*/) noexcept
    {
        // Return a fixed point matching typical target used in the test
        return Vector3<float>{100.f, 0.f, 0.f};
    }
    static float calc_vector_2d_distance(const Vector3<float>& v) noexcept { return std::hypot(v.x, v.y); }
    static float get_vector_height_coordinate(const Vector3<float>& v) noexcept { return v.z; }
    static Vector3<float> calc_viewpoint_from_angles(const Projectile& /*p*/, Vector3<float> /*v*/, std::optional<float> /*maybe_pitch*/) noexcept
    {
        return Vector3<float>{1.f, 2.f, 3.f};
    }
    static float calc_direct_pitch_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept { return 12.5f; }
    static float calc_direct_yaw_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept { return 0.f; }
};

TEST(ProjPredLegacyMore, ZeroGravityUsesDirectPitchAndReturnsViewpoint)
{
    const Projectile proj{ .m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 10.f, .m_gravity_scale = 0.f };
    const Target target{ .m_origin = {100.f, 0.f, 0.f}, .m_velocity = {0.f,0.f,0.f}, .m_is_airborne = false };

    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<FakeEngineZeroGravity>;
    const Engine engine(9.8f, 0.1f, 5.f, 1e-3f);

    const auto res = engine.maybe_calculate_aim_point(proj, target);
    ASSERT_TRUE(res.has_value());
    const auto v = res.value();
    EXPECT_NEAR(v.x, 1.f, 1e-6f);
    EXPECT_NEAR(v.y, 2.f, 1e-6f);
    EXPECT_NEAR(v.z, 3.f, 1e-6f);
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
    const Projectile proj{ .m_origin = {0.f,0.f,0.f}, .m_launch_speed = 1.f, .m_gravity_scale = 1.f };
    const Target target{ .m_origin = {10000.f, 0.f, 0.f}, .m_velocity = {0.f,0.f,0.f}, .m_is_airborne = false };

    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<FakeEngineNoSolution>;
    const Engine engine(9.8f, 0.5f, 2.f, 1.f);

    const auto res = engine.maybe_calculate_aim_point(proj, target);
    EXPECT_FALSE(res.has_value());
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
    const Projectile proj{ .m_origin = {0.f,0.f,0.f}, .m_launch_speed = 100.f, .m_gravity_scale = 1.f };
    const Target target{ .m_origin = {10.f, 0.f, 0.f}, .m_velocity = {0.f,0.f,0.f}, .m_is_airborne = false };

    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<FakeEngineAngleButMiss>;
    const Engine engine(9.8f, 0.1f, 1.f, 0.1f);

    const auto res = engine.maybe_calculate_aim_point(proj, target);
    EXPECT_FALSE(res.has_value());
}
