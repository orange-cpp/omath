#include <gtest/gtest.h>
#include <omath/engines/source_engine/traits/camera_trait.hpp>
#include <omath/engines/source_engine/traits/pred_engine_trait.hpp>
#include <omath/projectile_prediction/proj_pred_engine_avx2.hpp>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>
#include <stdexcept>

using Projectile = omath::projectile_prediction::Projectile<float>;
using Target = omath::projectile_prediction::Target<float>;
using Launcher = omath::projectile_prediction::Launcher<float>;
using Avx2Engine = omath::projectile_prediction::ProjPredEngineAvx2;

#if defined(OMATH_USE_AVX2) && (defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86))

// Fire from where the solution says the muzzle is, with the solution's angles, and see how close the projectile gets
// to the predicted target at the solution's time of flight. The AVX2 solver picks the first feasible time step, so the
// miss can be up to one step of travel.
static void expect_avx2_solution_hits(const Projectile& proj, const Launcher& launcher, const Target& target,
                                      const float gravity, const float step, const float max_time,
                                      const float hit_tolerance)
{
    using Trait = omath::source_engine::PredEngineTrait;
    const Avx2Engine engine(gravity, step, max_time);

    const auto solution = engine.maybe_calculate_aim(proj, launcher, target);
    ASSERT_TRUE(solution.has_value()) << "AVX2 engine must find a solution";

    const auto& [pitch, yaw] = solution->angles;
    const auto launch_origin = launcher.launch_origin(Trait::calc_view_basis(pitch, yaw));
    const auto proj_pos =
            Trait::predict_projectile_position(launch_origin, proj, pitch, yaw, solution->time_of_flight, gravity);
    const auto tgt_pos = Trait::predict_target_position(target, solution->time_of_flight, gravity);

    EXPECT_LE(proj_pos.distance_to(tgt_pos), hit_tolerance)
            << "closest approach " << proj_pos.distance_to(tgt_pos) << " at t=" << solution->time_of_flight;

    const auto [cam_pitch, cam_yaw, cam_roll] =
            omath::source_engine::CameraTrait::calc_look_at_angle(launcher.eye_origin, solution->aim_point);
    EXPECT_NEAR(pitch, -cam_pitch.as_degrees(), 0.01f);
    EXPECT_NEAR(yaw, cam_yaw.as_degrees(), 0.01f);
}

TEST(ProjPredEngineAvx2, StaticTarget)
{
    constexpr Target target{.m_origin = {300, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 2000.f, .m_gravity_scale = 0.4f};
    constexpr Launcher launcher{.eye_origin = {3, 2, 1}};

    expect_avx2_solution_hits(proj, launcher, target, 400.f, 1.f / 1000.f, 50.f, 5.f);
}

TEST(ProjPredEngineAvx2, MovingTarget)
{
    constexpr Target target{.m_origin = {500, 100, 0}, .m_velocity = {-50, 20, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 1500.f, .m_gravity_scale = 1.0f};
    constexpr Launcher launcher{.eye_origin = {0, 0, 64}};

    expect_avx2_solution_hits(proj, launcher, target, 800.f, 1.f / 1000.f, 30.f, 5.f);
}

TEST(ProjPredEngineAvx2, AirborneTargetWithMuzzleOffset)
{
    constexpr Target target{.m_origin = {200, 50, 300}, .m_velocity = {10, -5, -20}, .m_is_airborne = true};
    constexpr Projectile proj = {.m_launch_speed = 1800.f, .m_gravity_scale = 0.5f};
    constexpr Launcher launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = {.forward = 16.f, .right = 8.f, .up = -6.f}};

    expect_avx2_solution_hits(proj, launcher, target, 400.f, 1.f / 1000.f, 50.f, 6.f);
}

TEST(ProjPredEngineAvx2, AgreesWithLegacyEngine)
{
    constexpr Target target{.m_origin = {400, -150, 30}, .m_velocity = {-30, 10, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 2500.f, .m_gravity_scale = 0.6f};
    constexpr Launcher launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = {.forward = 16.f, .right = 8.f, .up = -6.f}};

    const auto avx2 = Avx2Engine(400.f, 1.f / 1000.f, 50.f).maybe_calculate_aim(proj, launcher, target);
    const auto legacy = omath::projectile_prediction::ProjPredEngineLegacy<>(400.f, 1.f / 1000.f, 50.f, 5.f)
                                .maybe_calculate_aim(proj, launcher, target);
    ASSERT_TRUE(avx2.has_value());
    ASSERT_TRUE(legacy.has_value());

    EXPECT_NEAR(avx2->angles.pitch, legacy->angles.pitch, 0.25f);
    EXPECT_NEAR(avx2->angles.yaw, legacy->angles.yaw, 0.25f);
    EXPECT_NEAR(avx2->time_of_flight, legacy->time_of_flight, 3.f / 1000.f);
}

TEST(ProjPredEngineAvx2, OutOfRangeReturnsNullopt)
{
    constexpr Target target{.m_origin = {100000, 0, 0}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 1.f, .m_gravity_scale = 1.f};

    const Avx2Engine engine(400.f, 1.f / 1000.f, 5.f);
    EXPECT_FALSE(engine.maybe_calculate_aim(proj, Launcher{.eye_origin = {0, 0, 0}}, target).has_value());
    EXPECT_FALSE(engine.maybe_calculate_aim_point(proj, target).has_value());
}

#else

TEST(ProjPredEngineAvx2, ThrowsWhenAvx2IsNotCompiledIn)
{
    constexpr Target target{.m_origin = {300, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile proj = {.m_launch_speed = 2000.f, .m_gravity_scale = 0.4f};

    const Avx2Engine engine(400.f, 1.f / 1000.f, 50.f);
    EXPECT_THROW(std::ignore = engine.maybe_calculate_aim(proj, Launcher{.eye_origin = {3, 2, 1}}, target),
                 std::runtime_error);
    EXPECT_THROW(std::ignore = engine.maybe_calculate_aim_point(proj, target), std::runtime_error);
}

#endif
