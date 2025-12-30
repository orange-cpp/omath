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
    const auto proj = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(60.f), {.m_width=1280.f, .m_height=720.f}, 0.1f, 1000.f);
    const auto expected = e::calc_perspective_projection_matrix(60.f, 1280.f / 720.f, 0.1f, 1000.f);
    expect_matrix_near(proj, expected);
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

    const auto proj = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(45.f), {.m_width=1920.f, .m_height=1080.f}, 0.1f, 1000.f);
    const auto expected = e::calc_perspective_projection_matrix(45.f, 1920.f / 1080.f, 0.1f, 1000.f);
    expect_matrix_near(proj, expected);

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

    const auto proj = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(60.f), {.m_width=1280.f, .m_height=720.f}, 0.1f, 1000.f);
    const auto expected = e::calc_perspective_projection_matrix(60.f, 1280.f / 720.f, 0.1f, 1000.f);
    expect_matrix_near(proj, expected);

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

    const auto proj = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(60.f), {.m_width=1280.f, .m_height=720.f}, 0.1f, 1000.f);
    const auto expected = e::calc_perspective_projection_matrix(60.f, 1280.f / 720.f, 0.1f, 1000.f);
    expect_matrix_near(proj, expected);

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

    const auto proj = e::CameraTrait::calc_projection_matrix(projection::FieldOfView::from_degrees(60.f), {.m_width=1280.f, .m_height=720.f}, 0.1f, 1000.f);
    const auto expected = e::calc_perspective_projection_matrix(60.f, 1280.f / 720.f, 0.1f, 1000.f);
    expect_matrix_near(proj, expected);

    // non-airborne
    t.m_is_airborne = false;
    const auto pred_ground_unreal = e::PredEngineTrait::predict_target_position(t, 2.f, 9.81f);
    EXPECT_NEAR(pred_ground_unreal.x, 4.f, 1e-6f);
}
