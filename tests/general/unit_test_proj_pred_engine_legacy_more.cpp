#include <gtest/gtest.h>
#include <omath/linear_algebra/vector3.hpp>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>
#include <omath/projectile_prediction/projectile.hpp>
#include <omath/projectile_prediction/target.hpp>

using Projectile = omath::projectile_prediction::Projectile<float>;
using Target = omath::projectile_prediction::Target<float>;
using ViewBasis = omath::projectile_prediction::ViewBasis<float>;
using omath::Vector3;

// World axes for every angle: enough for the engine to build launch origins and aim points from
static ViewBasis fake_basis() noexcept
{
    return {.forward = {1.f, 0.f, 0.f}, .right = {0.f, 1.f, 0.f}, .up = {0.f, 0.f, 1.f}};
}

// Fake engine trait where gravity is effectively zero and projectile prediction always hits the target
struct FakeEngineZeroGravity
{
    static Vector3<float> predict_target_position(const Target& t, float /*time*/, float /*gravity*/) noexcept
    {
        return t.m_origin;
    }
    static Vector3<float> predict_projectile_position(const Vector3<float>& /*launch_origin*/, const Projectile& /*p*/,
                                                      float /*pitch*/, float /*yaw*/, float /*time*/,
                                                      float /*gravity*/) noexcept
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
    static ViewBasis calc_view_basis(float /*pitch*/, float /*yaw*/) noexcept
    {
        return fake_basis();
    }
    static float calc_direct_pitch_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept
    {
        return 12.5f;
    }
    static float calc_direct_yaw_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept
    {
        return 0.f;
    }
};

TEST(ProjPredLegacyMore, ZeroGravityUsesDirectPitchAndReturnsAimPointOnForwardRay)
{
    constexpr Projectile proj{.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 10.f, .m_gravity_scale = 0.f};
    constexpr Target target{.m_origin = {100.f, 0.f, 0.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false};

    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<FakeEngineZeroGravity>;
    const Engine engine(9.8f, 0.1f, 5.f, 1e-3f);

    const auto res = engine.maybe_calculate_aim_point(proj, target);
    ASSERT_TRUE(res.has_value());
    // eye + fake forward * distance(eye, target)
    const auto v = res.value();
    EXPECT_NEAR(v.x, 100.f, 1e-4f);
    EXPECT_NEAR(v.y, 0.f, 1e-6f);
    EXPECT_NEAR(v.z, 0.f, 1e-6f);
}

TEST(ProjPredLegacyMore, ZeroGravityAimAnglesReturnsPitchAndYaw)
{
    constexpr Projectile proj{.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 10.f, .m_gravity_scale = 0.f};
    constexpr Target target{.m_origin = {100.f, 0.f, 0.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false};

    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<FakeEngineZeroGravity>;
    const Engine engine(9.8f, 0.1f, 5.f, 1e-3f);

    const auto res = engine.maybe_calculate_aim_angles(proj, target);
    ASSERT_TRUE(res.has_value());
    // FakeEngineZeroGravity::calc_direct_pitch_angle returns 12.5f
    EXPECT_NEAR(res->pitch, 12.5f, 1e-6f);
    // FakeEngineZeroGravity::calc_direct_yaw_angle returns 0.f
    EXPECT_NEAR(res->yaw, 0.f, 1e-6f);
}

TEST(ProjPredLegacyMore, SolutionCarriesTargetAndTimeOfFlight)
{
    constexpr Projectile proj{.m_launch_speed = 10.f, .m_gravity_scale = 0.f};
    constexpr omath::projectile_prediction::Launcher<float> launcher{.eye_origin = {0.f, 0.f, 0.f}};
    constexpr Target target{.m_origin = {100.f, 0.f, 0.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false};

    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<FakeEngineZeroGravity>;
    const Engine engine(9.8f, 0.1f, 5.f, 1e-3f);

    const auto res = engine.maybe_calculate_aim(proj, launcher, target);
    ASSERT_TRUE(res.has_value());
    EXPECT_FLOAT_EQ(res->time_of_flight, 0.f); // the fake hits at the very first step
    EXPECT_NEAR(res->predicted_target_position.x, 100.f, 1e-6f);
    EXPECT_NEAR(res->angles.pitch, 12.5f, 1e-6f);
}

// Fake trait producing no valid launch angle (root < 0)
struct FakeEngineNoSolution
{
    static Vector3<float> predict_target_position(const Target& t, float /*time*/, float /*gravity*/) noexcept
    {
        return t.m_origin;
    }
    static Vector3<float> predict_projectile_position(const Vector3<float>& /*launch_origin*/, const Projectile& /*p*/,
                                                      float /*pitch*/, float /*yaw*/, float /*time*/,
                                                      float /*gravity*/) noexcept
    {
        return Vector3<float>{0.f, 0.f, 0.f};
    }
    static float calc_vector_2d_distance(const Vector3<float>& /*v*/) noexcept
    {
        return 10000.f;
    }
    static float get_vector_height_coordinate(const Vector3<float>& /*v*/) noexcept
    {
        return 0.f;
    }
    static ViewBasis calc_view_basis(float /*pitch*/, float /*yaw*/) noexcept
    {
        return fake_basis();
    }
    static float calc_direct_pitch_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept
    {
        return 0.f;
    }
    static float calc_direct_yaw_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept
    {
        return 0.f;
    }
};

TEST(ProjPredLegacyMore, NoSolutionRootReturnsNullopt)
{
    // Very slow projectile and large distance -> quadratic root negative
    constexpr Projectile proj{.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 1.f, .m_gravity_scale = 1.f};
    constexpr Target target{.m_origin = {10000.f, 0.f, 0.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false};

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
    static Vector3<float> predict_target_position(const Target& t, float /*time*/, float /*gravity*/) noexcept
    {
        return t.m_origin;
    }
    static Vector3<float> predict_projectile_position(const Vector3<float>& /*launch_origin*/, const Projectile& /*p*/,
                                                      float /*pitch*/, float /*yaw*/, float /*time*/,
                                                      float /*gravity*/) noexcept
    {
        // always return a point far from the target
        return Vector3<float>{0.f, 0.f, 1000.f};
    }
    static float calc_vector_2d_distance(const Vector3<float>& v) noexcept
    {
        return std::hypot(v.x, v.y);
    }
    static float get_vector_height_coordinate(const Vector3<float>& v) noexcept
    {
        return v.z;
    }
    static ViewBasis calc_view_basis(float /*pitch*/, float /*yaw*/) noexcept
    {
        return fake_basis();
    }
    static float calc_direct_pitch_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept
    {
        return 1.f;
    }
    static float calc_direct_yaw_angle(const Vector3<float>& /*a*/, const Vector3<float>& /*b*/) noexcept
    {
        return 0.f;
    }
};

TEST(ProjPredLegacyMore, AngleComputedButMissReturnsNullopt)
{
    constexpr Projectile proj{.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 100.f, .m_gravity_scale = 1.f};
    constexpr Target target{.m_origin = {10.f, 0.f, 0.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false};

    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<FakeEngineAngleButMiss>;
    const Engine engine(9.8f, 0.1f, 1.f, 0.1f);

    const auto res = engine.maybe_calculate_aim_point(proj, target);
    EXPECT_FALSE(res.has_value());
}
