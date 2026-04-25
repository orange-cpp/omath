// Tests for engine trait headers to improve header coverage
#include <gtest/gtest.h>
#include <omath/engines/frostbite_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/frostbite_engine/traits/mesh_trait.hpp>
#include <omath/engines/frostbite_engine/traits/camera_trait.hpp>

#include <omath/engines/iw_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/iw_engine/traits/mesh_trait.hpp>
#include <omath/engines/iw_engine/traits/camera_trait.hpp>

#include <omath/engines/opengl_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/opengl_engine/traits/mesh_trait.hpp>
#include <omath/engines/opengl_engine/traits/camera_trait.hpp>

#include <omath/engines/unity_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/unity_engine/traits/mesh_trait.hpp>
#include <omath/engines/unity_engine/traits/camera_trait.hpp>

#include <omath/engines/unreal_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/unreal_engine/traits/mesh_trait.hpp>
#include <omath/engines/unreal_engine/traits/camera_trait.hpp>

#include <omath/engines/source_engine/traits/pred_engine_trait.hpp>
#include <omath/engines/source_engine/traits/camera_trait.hpp>

#include <omath/engines/cry_engine/traits/camera_trait.hpp>

#include <omath/projectile_prediction/projectile.hpp>
#include <omath/projectile_prediction/target.hpp>
#include <optional>

using namespace omath;

// Small helper to compare matrices roughly (templated to avoid concrete typedef)
template<typename MatT>
static void expect_matrix_near(const MatT& a, const MatT& b, float eps = 1e-5f)
{
    for (std::size_t r = 0; r < 4; ++r)
        for (std::size_t c = 0; c < 4; ++c)
            EXPECT_NEAR(a.at(r, c), b.at(r, c), eps);
}

// ── Launch offset tests for all engines ──────────────────────────────────────
#include <omath/engines/cry_engine/traits/pred_engine_trait.hpp>

// Helper: verify that zero offset matches default-initialized offset behavior
template<typename Trait>
static void verify_launch_offset_at_time_zero(const Vector3<float>& origin, const Vector3<float>& offset)
{
    projectile_prediction::Projectile p;
    p.m_origin = origin;
    p.m_launch_offset = offset;
    p.m_launch_speed = 100.f;
    p.m_gravity_scale = 1.f;

    const auto pos = Trait::predict_projectile_position(p, 0.f, 0.f, 0.f, 9.81f);
    const auto expected = origin + offset;
    EXPECT_NEAR(pos.x, expected.x, 1e-4f);
    EXPECT_NEAR(pos.y, expected.y, 1e-4f);
    EXPECT_NEAR(pos.z, expected.z, 1e-4f);
}

template<typename Trait>
static void verify_zero_offset_matches_default()
{
    projectile_prediction::Projectile p;
    p.m_origin = {10.f, 20.f, 30.f};
    p.m_launch_offset = {0.f, 0.f, 0.f};
    p.m_launch_speed = 50.f;
    p.m_gravity_scale = 1.f;

    projectile_prediction::Projectile p2;
    p2.m_origin = {10.f, 20.f, 30.f};
    p2.m_launch_speed = 50.f;
    p2.m_gravity_scale = 1.f;

    const auto pos1 = Trait::predict_projectile_position(p, 15.f, 30.f, 1.f, 9.81f);
    const auto pos2 = Trait::predict_projectile_position(p2, 15.f, 30.f, 1.f, 9.81f);
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(_M_ARM64)
    constexpr float tol = 1e-6f;
#else
    constexpr float tol = 1e-4f;
#endif
    EXPECT_NEAR(pos1.x, pos2.x, tol);
    EXPECT_NEAR(pos1.y, pos2.y, tol);
    EXPECT_NEAR(pos1.z, pos2.z, tol);
}

TEST(LaunchOffsetTests, Source_OffsetAtTimeZero)
{
    verify_launch_offset_at_time_zero<source_engine::PredEngineTrait>({0, 0, 0}, {5, 3, -2});
}
TEST(LaunchOffsetTests, Source_ZeroOffsetMatchesDefault)
{
    verify_zero_offset_matches_default<source_engine::PredEngineTrait>();
}
TEST(LaunchOffsetTests, Frostbite_OffsetAtTimeZero)
{
    verify_launch_offset_at_time_zero<frostbite_engine::PredEngineTrait>({0, 0, 0}, {5, 3, -2});
}
TEST(LaunchOffsetTests, Frostbite_ZeroOffsetMatchesDefault)
{
    verify_zero_offset_matches_default<frostbite_engine::PredEngineTrait>();
}
TEST(LaunchOffsetTests, IW_OffsetAtTimeZero)
{
    verify_launch_offset_at_time_zero<iw_engine::PredEngineTrait>({0, 0, 0}, {5, 3, -2});
}
TEST(LaunchOffsetTests, IW_ZeroOffsetMatchesDefault)
{
    verify_zero_offset_matches_default<iw_engine::PredEngineTrait>();
}
TEST(LaunchOffsetTests, OpenGL_OffsetAtTimeZero)
{
    verify_launch_offset_at_time_zero<opengl_engine::PredEngineTrait>({0, 0, 0}, {5, 3, -2});
}
TEST(LaunchOffsetTests, OpenGL_ZeroOffsetMatchesDefault)
{
    verify_zero_offset_matches_default<opengl_engine::PredEngineTrait>();
}
TEST(LaunchOffsetTests, Unity_OffsetAtTimeZero)
{
    verify_launch_offset_at_time_zero<unity_engine::PredEngineTrait>({0, 0, 0}, {5, 3, -2});
}
TEST(LaunchOffsetTests, Unity_ZeroOffsetMatchesDefault)
{
    verify_zero_offset_matches_default<unity_engine::PredEngineTrait>();
}
TEST(LaunchOffsetTests, Unreal_OffsetAtTimeZero)
{
    verify_launch_offset_at_time_zero<unreal_engine::PredEngineTrait>({0, 0, 0}, {5, 3, -2});
}
TEST(LaunchOffsetTests, Unreal_ZeroOffsetMatchesDefault)
{
    verify_zero_offset_matches_default<unreal_engine::PredEngineTrait>();
}
TEST(LaunchOffsetTests, CryEngine_OffsetAtTimeZero)
{
    verify_launch_offset_at_time_zero<cry_engine::PredEngineTrait>({0, 0, 0}, {5, 3, -2});
}
TEST(LaunchOffsetTests, CryEngine_ZeroOffsetMatchesDefault)
{
    verify_zero_offset_matches_default<cry_engine::PredEngineTrait>();
}

// Test that offset shifts the projectile position at t>0 as well
TEST(LaunchOffsetTests, OffsetShiftsTrajectory)
{
    projectile_prediction::Projectile p_no_offset;
    p_no_offset.m_origin = {0.f, 0.f, 0.f};
    p_no_offset.m_launch_speed = 100.f;
    p_no_offset.m_gravity_scale = 1.f;

    projectile_prediction::Projectile p_with_offset;
    p_with_offset.m_origin = {0.f, 0.f, 0.f};
    p_with_offset.m_launch_offset = {10.f, 5.f, -3.f};
    p_with_offset.m_launch_speed = 100.f;
    p_with_offset.m_gravity_scale = 1.f;

    const auto pos1 = source_engine::PredEngineTrait::predict_projectile_position(p_no_offset, 20.f, 45.f, 2.f, 9.81f);
    const auto pos2 = source_engine::PredEngineTrait::predict_projectile_position(p_with_offset, 20.f, 45.f, 2.f, 9.81f);

    // The difference should be exactly the launch offset
    EXPECT_NEAR(pos2.x - pos1.x, 10.f, 1e-4f);
    EXPECT_NEAR(pos2.y - pos1.y, 5.f, 1e-4f);
    EXPECT_NEAR(pos2.z - pos1.z, -3.f, 1e-4f);
}

// Generic tests for PredEngineTrait behaviour across engines
TEST(TraitTests, Frostbite_Pred_And_Mesh_And_Camera)
{
    namespace e = omath::frostbite_engine;

    projectile_prediction::Projectile p;
    p.m_origin = {0.f, 0.f, 0.f};
    p.m_launch_speed = 10.f;
    p.m_gravity_scale = 1.f;

    const auto pos = e::PredEngineTrait::predict_projectile_position(p, 0.f, 0.f, 1.f, 9.81f);
    EXPECT_NEAR(pos.x, 0.f, 1e-4f);
    EXPECT_NEAR(pos.z, 10.f, 1e-4f);
    EXPECT_NEAR(pos.y, -9.81f * 0.5f, 1e-4f);

    projectile_prediction::Target t;
    t.m_origin = {0.f, 5.f, 0.f};
    t.m_velocity = {2.f, 0.f, 0.f};
    t.m_is_airborne = true;
    const auto pred = e::PredEngineTrait::predict_target_position(t, 2.f, 9.81f);
    EXPECT_NEAR(pred.x, 4.f, 1e-6f);
    EXPECT_NEAR(pred.y, 5.f - 9.81f * (2.f * 2.f) * 0.5f, 1e-6f);

    // Also test non-airborne path (no gravity applied)
    t.m_is_airborne = false;
    const auto pred_ground = e::PredEngineTrait::predict_target_position(t, 2.f, 9.81f);
    EXPECT_NEAR(pred_ground.x, 4.f, 1e-6f);
    EXPECT_NEAR(pred_ground.y, 5.f, 1e-6f);

    EXPECT_NEAR(e::PredEngineTrait::calc_vector_2d_distance({3.f, 0.f, 4.f}), 5.f, 1e-6f);
    EXPECT_NEAR(e::PredEngineTrait::get_vector_height_coordinate({1.f, 2.5f, 3.f}), 2.5f, 1e-6f);

    std::optional<float> pitch = 45.f;
    auto vp = e::PredEngineTrait::calc_viewpoint_from_angles(p, {10.f, 0.f, 0.f}, pitch);
    EXPECT_NEAR(vp.y, 0.f + 10.f * std::tan(angles::degrees_to_radians(45.f)), 1e-6f);

    // Direct angles
    Vector3<float> origin{0.f, 0.f, 0.f};
    Vector3<float> view_to{0.f, 1.f, 1.f};
    const auto pitch_calc = e::PredEngineTrait::calc_direct_pitch_angle(origin, view_to);
    const auto dir = (view_to - origin).normalized();
    EXPECT_NEAR(pitch_calc, angles::radians_to_degrees(std::asin(dir.y)), 1e-3f);

    const auto yaw_calc = e::PredEngineTrait::calc_direct_yaw_angle(origin, view_to);
    EXPECT_NEAR(yaw_calc, angles::radians_to_degrees(std::atan2(dir.x, dir.z)), 1e-3f);

    // MeshTrait simply forwards to rotation_matrix; ensure it compiles and returns something
    e::ViewAngles va;
    const auto m1 = e::MeshTrait::rotation_matrix(va);
    const auto m2 = e::rotation_matrix(va);
    expect_matrix_near(m1, m2);

    // CameraTrait look at should be callable
    const auto angles = e::CameraTrait::calc_look_at_angle({0, 0, 0}, {0, 1, 1});
    (void)angles;
    const auto proj = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(60.f), {1280.f, 720.f}, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);
    const auto expected = e::calc_perspective_projection_matrix(60.f, 1280.f / 720.f, 0.1f, 1000.f);
    expect_matrix_near(proj, expected);

    const auto proj_zo = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(60.f), {1280.f, 720.f}, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);
    const auto expected_zo = e::calc_perspective_projection_matrix(60.f, 1280.f / 720.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);
    expect_matrix_near(proj_zo, expected_zo);
    EXPECT_NE(proj, proj_zo);
}

TEST(TraitTests, IW_Pred_And_Mesh_And_Camera)
{
    namespace e = omath::iw_engine;

    projectile_prediction::Projectile p;
    p.m_origin = {0.f, 0.f, 0.f};
    p.m_launch_speed = 10.f;
    p.m_gravity_scale = 1.f;

    const auto pos = e::PredEngineTrait::predict_projectile_position(p, 0.f, 0.f, 1.f, 9.81f);
    EXPECT_NEAR(pos.x, 10.f, 1e-4f);
    EXPECT_NEAR(pos.z, -9.81f * 0.5f, 1e-4f);

    projectile_prediction::Target t;
    t.m_origin = {0.f, 0.f, 5.f};
    t.m_velocity = {0.f, 0.f, 2.f};
    t.m_is_airborne = true;
    const auto pred = e::PredEngineTrait::predict_target_position(t, 2.f, 9.81f);
    // predicted = origin + velocity * t -> z = 5 + 2*2 = 9; then gravity applied
    EXPECT_NEAR(pred.z, 9.f - 9.81f * (2.f * 2.f) * 0.5f, 1e-6f);

    EXPECT_NEAR(e::PredEngineTrait::calc_vector_2d_distance({3.f, 4.f, 0.f}), 5.f, 1e-6f);
    EXPECT_NEAR(e::PredEngineTrait::get_vector_height_coordinate({1.f, 2.5f, 3.f}), 3.f, 1e-6f);

    std::optional<float> pitch = 45.f;
    auto vp = e::PredEngineTrait::calc_viewpoint_from_angles(p, {10.f, 0.f, 0.f}, pitch);
    EXPECT_NEAR(vp.z, 0.f + 10.f * std::tan(angles::degrees_to_radians(45.f)), 1e-6f);

    Vector3<float> origin{0.f, 0.f, 0.f};
    Vector3<float> view_to{1.f, 1.f, 1.f};
    const auto pitch_calc = e::PredEngineTrait::calc_direct_pitch_angle(origin, view_to);
    const auto dist = origin.distance_to(view_to);
    EXPECT_NEAR(pitch_calc, angles::radians_to_degrees(std::asin((view_to.z - origin.z) / dist)), 1e-3f);

    const auto yaw_calc = e::PredEngineTrait::calc_direct_yaw_angle(origin, view_to);
    const auto delta = view_to - origin;
    EXPECT_NEAR(yaw_calc, angles::radians_to_degrees(std::atan2(delta.y, delta.x)), 1e-3f);

    e::ViewAngles va;
    expect_matrix_near(e::MeshTrait::rotation_matrix(va), e::rotation_matrix(va));

    const auto proj = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(45.f), {1920.f, 1080.f}, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);
    const auto expected = e::calc_perspective_projection_matrix(45.f, 1920.f / 1080.f, 0.1f, 1000.f);
    expect_matrix_near(proj, expected);

    const auto proj_zo = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(45.f), {1920.f, 1080.f}, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);
    const auto expected_zo = e::calc_perspective_projection_matrix(45.f, 1920.f / 1080.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);
    expect_matrix_near(proj_zo, expected_zo);
    EXPECT_NE(proj, proj_zo);

    // non-airborne
    t.m_is_airborne = false;
    const auto pred_ground_iw = e::PredEngineTrait::predict_target_position(t, 2.f, 9.81f);
    EXPECT_NEAR(pred_ground_iw.z, 9.f, 1e-6f);
}

TEST(TraitTests, OpenGL_Pred_And_Mesh_And_Camera)
{
    namespace e = omath::opengl_engine;

    projectile_prediction::Projectile p;
    p.m_origin = {0.f, 0.f, 0.f};
    p.m_launch_speed = 10.f;
    p.m_gravity_scale = 1.f;

    const auto pos = e::PredEngineTrait::predict_projectile_position(p, 0.f, 0.f, 1.f, 9.81f);
    EXPECT_NEAR(pos.z, -10.f, 1e-4f);
    EXPECT_NEAR(pos.y, -9.81f * 0.5f, 1e-4f);

    projectile_prediction::Target t;
    t.m_origin = {0.f, 5.f, 0.f};
    t.m_velocity = {2.f, 0.f, 0.f};
    t.m_is_airborne = true;
    const auto pred = e::PredEngineTrait::predict_target_position(t, 2.f, 9.81f);
    EXPECT_NEAR(pred.x, 4.f, 1e-6f);
    EXPECT_NEAR(pred.y, 5.f - 9.81f * (2.f * 2.f) * 0.5f, 1e-6f);

    EXPECT_NEAR(e::PredEngineTrait::calc_vector_2d_distance({3.f, 0.f, 4.f}), 5.f, 1e-6f);
    EXPECT_NEAR(e::PredEngineTrait::get_vector_height_coordinate({1.f, 2.5f, 3.f}), 2.5f, 1e-6f);

    std::optional<float> pitch = 45.f;
    auto vp = e::PredEngineTrait::calc_viewpoint_from_angles(p, {10.f, 0.f, 0.f}, pitch);
    EXPECT_NEAR(vp.y, 0.f + 10.f * std::tan(angles::degrees_to_radians(45.f)), 1e-6f);

    Vector3<float> origin{0.f, 0.f, 0.f};
    Vector3<float> view_to{0.f, 1.f, 1.f};
    const auto pitch_calc = e::PredEngineTrait::calc_direct_pitch_angle(origin, view_to);
    const auto dir = (view_to - origin).normalized();
    EXPECT_NEAR(pitch_calc, angles::radians_to_degrees(std::asin(dir.y)), 1e-3f);

    const auto yaw_calc = e::PredEngineTrait::calc_direct_yaw_angle(origin, view_to);
    EXPECT_NEAR(yaw_calc, angles::radians_to_degrees(-std::atan2(dir.x, -dir.z)), 1e-3f);

    e::ViewAngles va;
    expect_matrix_near(e::MeshTrait::rotation_matrix(va), e::rotation_matrix(va));

    const auto proj = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(60.f), {1280.f, 720.f}, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);
    const auto expected = e::calc_perspective_projection_matrix(60.f, 1280.f / 720.f, 0.1f, 1000.f);
    expect_matrix_near(proj, expected);

    const auto proj_zo = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(60.f), {1280.f, 720.f}, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);
    const auto expected_zo = e::calc_perspective_projection_matrix(60.f, 1280.f / 720.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);
    expect_matrix_near(proj_zo, expected_zo);
    EXPECT_NE(proj, proj_zo);

    // non-airborne
    t.m_is_airborne = false;
    const auto pred_ground_gl = e::PredEngineTrait::predict_target_position(t, 2.f, 9.81f);
    EXPECT_NEAR(pred_ground_gl.x, 4.f, 1e-6f);
}

TEST(TraitTests, Unity_Pred_And_Mesh_And_Camera)
{
    namespace e = omath::unity_engine;

    projectile_prediction::Projectile p;
    p.m_origin = {0.f, 0.f, 0.f};
    p.m_launch_speed = 10.f;
    p.m_gravity_scale = 1.f;

    const auto pos = e::PredEngineTrait::predict_projectile_position(p, 0.f, 0.f, 1.f, 9.81f);
    EXPECT_NEAR(pos.z, 10.f, 1e-4f);
    EXPECT_NEAR(pos.y, -9.81f * 0.5f, 1e-4f);

    projectile_prediction::Target t;
    t.m_origin = {0.f, 5.f, 0.f};
    t.m_velocity = {2.f, 0.f, 0.f};
    t.m_is_airborne = true;
    const auto pred = e::PredEngineTrait::predict_target_position(t, 2.f, 9.81f);
    EXPECT_NEAR(pred.x, 4.f, 1e-6f);
    EXPECT_NEAR(pred.y, 5.f - 9.81f * (2.f * 2.f) * 0.5f, 1e-6f);

    EXPECT_NEAR(e::PredEngineTrait::calc_vector_2d_distance({3.f, 0.f, 4.f}), 5.f, 1e-6f);
    EXPECT_NEAR(e::PredEngineTrait::get_vector_height_coordinate({1.f, 2.5f, 3.f}), 2.5f, 1e-6f);

    std::optional<float> pitch = 45.f;
    auto vp = e::PredEngineTrait::calc_viewpoint_from_angles(p, {10.f, 0.f, 0.f}, pitch);
    EXPECT_NEAR(vp.y, 0.f + 10.f * std::tan(angles::degrees_to_radians(45.f)), 1e-6f);

    Vector3<float> origin{0.f, 0.f, 0.f};
    Vector3<float> view_to{0.f, 1.f, 1.f};
    const auto pitch_calc = e::PredEngineTrait::calc_direct_pitch_angle(origin, view_to);
    const auto dir = (view_to - origin).normalized();
    EXPECT_NEAR(pitch_calc, angles::radians_to_degrees(std::asin(dir.y)), 1e-3f);

    const auto yaw_calc = e::PredEngineTrait::calc_direct_yaw_angle(origin, view_to);
    EXPECT_NEAR(yaw_calc, angles::radians_to_degrees(std::atan2(dir.x, dir.z)), 1e-3f);

    e::ViewAngles va;
    expect_matrix_near(e::MeshTrait::rotation_matrix(va), e::rotation_matrix(va));

    const auto proj = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(60.f), {1280.f, 720.f}, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);
    const auto expected = e::calc_perspective_projection_matrix(60.f, 1280.f / 720.f, 0.1f, 1000.f);
    expect_matrix_near(proj, expected);

    const auto proj_zo = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(60.f), {1280.f, 720.f}, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);
    const auto expected_zo = e::calc_perspective_projection_matrix(60.f, 1280.f / 720.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);
    expect_matrix_near(proj_zo, expected_zo);
    EXPECT_NE(proj, proj_zo);

    // non-airborne
    t.m_is_airborne = false;
    const auto pred_ground_unity = e::PredEngineTrait::predict_target_position(t, 2.f, 9.81f);
    EXPECT_NEAR(pred_ground_unity.x, 4.f, 1e-6f);
}

TEST(TraitTests, Unreal_Pred_And_Mesh_And_Camera)
{
    namespace e = omath::unreal_engine;

    projectile_prediction::Projectile p;
    p.m_origin = {0.f, 0.f, 0.f};
    p.m_launch_speed = 10.f;
    p.m_gravity_scale = 1.f;

    const auto pos = e::PredEngineTrait::predict_projectile_position(p, 0.f, 0.f, 1.f, 9.81f);
    EXPECT_NEAR(pos.x, 10.f, 1e-4f);
    EXPECT_NEAR(pos.y, -9.81f * 0.5f, 1e-4f);

    projectile_prediction::Target t;
    t.m_origin = {0.f, 5.f, 0.f};
    t.m_velocity = {2.f, 0.f, 0.f};
    t.m_is_airborne = true;
    const auto pred = e::PredEngineTrait::predict_target_position(t, 2.f, 9.81f);
    EXPECT_NEAR(pred.x, 4.f, 1e-6f);
    EXPECT_NEAR(pred.y, 5.f - 9.81f * (2.f * 2.f) * 0.5f, 1e-6f);

    EXPECT_NEAR(e::PredEngineTrait::calc_vector_2d_distance({3.f, 0.f, 4.f}), 5.f, 1e-6f);
    EXPECT_NEAR(e::PredEngineTrait::get_vector_height_coordinate({1.f, 2.5f, 3.f}), 2.5f, 1e-6f);

    std::optional<float> pitch = 45.f;
    auto vp = e::PredEngineTrait::calc_viewpoint_from_angles(p, {10.f, 0.f, 0.f}, pitch);
    EXPECT_NEAR(vp.z, 0.f + 10.f * std::tan(angles::degrees_to_radians(45.f)), 1e-6f);

    Vector3<float> origin{0.f, 0.f, 0.f};
    Vector3<float> view_to{1.f, 1.f, 1.f};
    const auto pitch_calc = e::PredEngineTrait::calc_direct_pitch_angle(origin, view_to);
    const auto dir = (view_to - origin).normalized();
    EXPECT_NEAR(pitch_calc, angles::radians_to_degrees(std::asin(dir.z)), 1e-3f);

    const auto yaw_calc = e::PredEngineTrait::calc_direct_yaw_angle(origin, view_to);
    EXPECT_NEAR(yaw_calc, angles::radians_to_degrees(std::atan2(dir.y, dir.x)), 1e-3f);

    e::ViewAngles va;
    expect_matrix_near(e::MeshTrait::rotation_matrix(va), e::rotation_matrix(va));

    const auto proj = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(60.f), {1280.f, 720.f}, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);
    const auto expected = e::calc_perspective_projection_matrix(60.f, 1280.f / 720.f, 0.1f, 1000.f);
    expect_matrix_near(proj, expected);

    const auto proj_zo = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(60.f), {1280.f, 720.f}, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);
    const auto expected_zo = e::calc_perspective_projection_matrix(60.f, 1280.f / 720.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);
    expect_matrix_near(proj_zo, expected_zo);
    EXPECT_NE(proj, proj_zo);

    // non-airborne
    t.m_is_airborne = false;
    const auto pred_ground_unreal = e::PredEngineTrait::predict_target_position(t, 2.f, 9.81f);
    EXPECT_NEAR(pred_ground_unreal.x, 4.f, 1e-6f);
}

// ── NDC Depth Range tests for Source and CryEngine camera traits ────────────

TEST(NDCDepthRangeTests, Source_BothDepthRanges)
{
    namespace e = omath::source_engine;

    const auto proj_no = e::CameraTrait::calc_projection_matrix(
            projection::FieldOfView::from_degrees(90.f), {1920.f, 1080.f}, 0.1f, 1000.f,
            NDCDepthRange::NEGATIVE_ONE_TO_ONE);
    const auto expected_no = e::calc_perspective_projection_matrix(
            90.f, 1920.f / 1080.f, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);
    expect_matrix_near(proj_no, expected_no);

    const auto proj_zo = e::CameraTrait::calc_projection_matrix(
            projection::FieldOfView::from_degrees(90.f), {1920.f, 1080.f}, 0.1f, 1000.f,
            NDCDepthRange::ZERO_TO_ONE);
    const auto expected_zo = e::calc_perspective_projection_matrix(
            90.f, 1920.f / 1080.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);
    expect_matrix_near(proj_zo, expected_zo);

    EXPECT_NE(proj_no, proj_zo);
}

TEST(NDCDepthRangeTests, CryEngine_BothDepthRanges)
{
    namespace e = omath::cry_engine;

    const auto proj_no = e::CameraTrait::calc_projection_matrix(
            projection::FieldOfView::from_degrees(90.f), {1920.f, 1080.f}, 0.1f, 1000.f,
            NDCDepthRange::NEGATIVE_ONE_TO_ONE);
    const auto expected_no = e::calc_perspective_projection_matrix(
            90.f, 1920.f / 1080.f, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);
    expect_matrix_near(proj_no, expected_no);

    const auto proj_zo = e::CameraTrait::calc_projection_matrix(
            projection::FieldOfView::from_degrees(90.f), {1920.f, 1080.f}, 0.1f, 1000.f,
            NDCDepthRange::ZERO_TO_ONE);
    const auto expected_zo = e::calc_perspective_projection_matrix(
            90.f, 1920.f / 1080.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);
    expect_matrix_near(proj_zo, expected_zo);

    EXPECT_NE(proj_no, proj_zo);
}

// ── Verify Z mapping for ZERO_TO_ONE across all engines ─────────────────────

// Helper: projects a point at given z through a left-handed projection matrix and returns NDC z
template<class Type = float, MatStoreType Store = MatStoreType::ROW_MAJOR>
static float project_z_lh(const Mat<4, 4, Type, Store>& proj, float z)
{
    auto clip = proj * mat_column_from_vector<Type, Store>({0, 0, static_cast<Type>(z)});
    return static_cast<float>(clip.at(2, 0) / clip.at(3, 0));
}

TEST(NDCDepthRangeTests, Source_ZeroToOne_ZRange)
{
    namespace e = omath::source_engine;
    // Source is left-handed
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);

    EXPECT_NEAR(project_z_lh(proj, 0.1f), 0.0f, 1e-4f);
    EXPECT_NEAR(project_z_lh(proj, 1000.f), 1.0f, 1e-4f);
    EXPECT_GT(project_z_lh(proj, 500.f), 0.0f);
    EXPECT_LT(project_z_lh(proj, 500.f), 1.0f);
}

TEST(NDCDepthRangeTests, IW_ZeroToOne_ZRange)
{
    namespace e = omath::iw_engine;
    // IW is left-handed
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);

    EXPECT_NEAR(project_z_lh(proj, 0.1f), 0.0f, 1e-4f);
    EXPECT_NEAR(project_z_lh(proj, 1000.f), 1.0f, 1e-4f);
    EXPECT_GT(project_z_lh(proj, 500.f), 0.0f);
    EXPECT_LT(project_z_lh(proj, 500.f), 1.0f);
}

TEST(NDCDepthRangeTests, OpenGL_ZeroToOne_ZRange)
{
    namespace e = omath::opengl_engine;
    // OpenGL is right-handed (negative z forward), column-major
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);

    // OpenGL engine uses column-major matrices, project manually
    auto proj_z = [&](float z) {
        auto clip = proj * mat_column_from_vector<float, MatStoreType::COLUMN_MAJOR>({0, 0, z});
        return clip.at(2, 0) / clip.at(3, 0);
    };

    EXPECT_NEAR(proj_z(-0.1f), 0.0f, 1e-4f);
    EXPECT_NEAR(proj_z(-1000.f), 1.0f, 1e-4f);
    EXPECT_GT(proj_z(-500.f), 0.0f);
    EXPECT_LT(proj_z(-500.f), 1.0f);
}

TEST(NDCDepthRangeTests, Frostbite_ZeroToOne_ZRange)
{
    namespace e = omath::frostbite_engine;
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);

    EXPECT_NEAR(project_z_lh(proj, 0.1f), 0.0f, 1e-4f);
    EXPECT_NEAR(project_z_lh(proj, 1000.f), 1.0f, 1e-4f);
    EXPECT_GT(project_z_lh(proj, 500.f), 0.0f);
    EXPECT_LT(project_z_lh(proj, 500.f), 1.0f);
}

TEST(NDCDepthRangeTests, Unity_ZeroToOne_ZRange)
{
    namespace e = omath::unity_engine;
    // Unity is right-handed, row-major
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);

    auto proj_z = [&](float z) {
        auto clip = proj * mat_column_from_vector<float>({0, 0, z});
        return clip.at(2, 0) / clip.at(3, 0);
    };

    EXPECT_NEAR(proj_z(-0.1f), 0.0f, 1e-4f);
    EXPECT_NEAR(proj_z(-1000.f), 1.0f, 1e-4f);
    EXPECT_GT(proj_z(-500.f), 0.0f);
    EXPECT_LT(proj_z(-500.f), 1.0f);
}

TEST(NDCDepthRangeTests, Unreal_ZeroToOne_ZRange)
{
    namespace e = omath::unreal_engine;
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);

    EXPECT_NEAR(project_z_lh(proj, 0.1f), 0.0f, 1e-4f);
    EXPECT_NEAR(project_z_lh(proj, 1000.f), 1.0f, 1e-4f);
    EXPECT_GT(project_z_lh(proj, 500.f), 0.0f);
    EXPECT_LT(project_z_lh(proj, 500.f), 1.0f);
}

TEST(NDCDepthRangeTests, CryEngine_ZeroToOne_ZRange)
{
    namespace e = omath::cry_engine;
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::ZERO_TO_ONE);

    EXPECT_NEAR(project_z_lh(proj, 0.1f), 0.0f, 1e-4f);
    EXPECT_NEAR(project_z_lh(proj, 1000.f), 1.0f, 1e-4f);
    EXPECT_GT(project_z_lh(proj, 500.f), 0.0f);
    EXPECT_LT(project_z_lh(proj, 500.f), 1.0f);
}

// ── Verify Z mapping for NEGATIVE_ONE_TO_ONE across all engines ─────────────

TEST(NDCDepthRangeTests, Source_NegativeOneToOne_ZRange)
{
    namespace e = omath::source_engine;
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);

    EXPECT_NEAR(project_z_lh(proj, 0.1f), -1.0f, 1e-3f);
    EXPECT_NEAR(project_z_lh(proj, 1000.f), 1.0f, 1e-3f);
}

TEST(NDCDepthRangeTests, IW_NegativeOneToOne_ZRange)
{
    namespace e = omath::iw_engine;
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);

    EXPECT_NEAR(project_z_lh(proj, 0.1f), -1.0f, 1e-3f);
    EXPECT_NEAR(project_z_lh(proj, 1000.f), 1.0f, 1e-3f);
}

TEST(NDCDepthRangeTests, Frostbite_NegativeOneToOne_ZRange)
{
    namespace e = omath::frostbite_engine;
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);

    EXPECT_NEAR(project_z_lh(proj, 0.1f), -1.0f, 1e-3f);
    EXPECT_NEAR(project_z_lh(proj, 1000.f), 1.0f, 1e-3f);
}

TEST(NDCDepthRangeTests, Unreal_NegativeOneToOne_ZRange)
{
    namespace e = omath::unreal_engine;
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);

    EXPECT_NEAR(project_z_lh(proj, 0.1f), -1.0f, 1e-3f);
    EXPECT_NEAR(project_z_lh(proj, 1000.f), 1.0f, 1e-3f);
}

TEST(NDCDepthRangeTests, CryEngine_NegativeOneToOne_ZRange)
{
    namespace e = omath::cry_engine;
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);

    EXPECT_NEAR(project_z_lh(proj, 0.1f), -1.0f, 1e-3f);
    EXPECT_NEAR(project_z_lh(proj, 1000.f), 1.0f, 1e-3f);
}

TEST(NDCDepthRangeTests, OpenGL_NegativeOneToOne_ZRange)
{
    namespace e = omath::opengl_engine;
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);

    auto proj_z = [&](float z) {
        auto clip = proj * mat_column_from_vector<float, MatStoreType::COLUMN_MAJOR>({0, 0, z});
        return clip.at(2, 0) / clip.at(3, 0);
    };

    EXPECT_NEAR(proj_z(-0.1f), -1.0f, 1e-3f);
    EXPECT_NEAR(proj_z(-1000.f), 1.0f, 1e-3f);
}

TEST(NDCDepthRangeTests, Unity_NegativeOneToOne_ZRange)
{
    namespace e = omath::unity_engine;
    const auto proj = e::calc_perspective_projection_matrix(90.f, 16.f / 9.f, 0.1f, 1000.f, NDCDepthRange::NEGATIVE_ONE_TO_ONE);

    auto proj_z = [&](float z) {
        auto clip = proj * mat_column_from_vector<float>({0, 0, z});
        return clip.at(2, 0) / clip.at(3, 0);
    };

    EXPECT_NEAR(proj_z(-0.1f), -1.0f, 1e-3f);
    EXPECT_NEAR(proj_z(-1000.f), 1.0f, 1e-3f);
}
