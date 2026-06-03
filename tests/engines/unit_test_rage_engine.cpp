//
// Created by Codex on 6/3/2026.
//
#include <gtest/gtest.h>
#include <omath/engines/rage_engine/camera.hpp>
#include <omath/engines/rage_engine/constants.hpp>
#include <omath/engines/rage_engine/formulas.hpp>
#include <omath/engines/rage_engine/traits/mesh_trait.hpp>
#include <omath/engines/rage_engine/traits/pred_engine_trait.hpp>
#include <omath/projectile_prediction/projectile.hpp>
#include <ranges>
#include <type_traits>

using namespace omath;

static_assert(std::is_same_v<rage_engine::Mat4X4::ContainedType, float>);

static void expect_rage_vector_near(const Vector3<float>& actual, const Vector3<float>& expected)
{
    for (const auto& [result, etalon] : std::views::zip(actual.as_array(), expected.as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}

TEST(unit_test_rage_engine, ForwardVector)
{
    const auto forward = rage_engine::forward_vector({});

    EXPECT_EQ(forward, rage_engine::k_abs_forward);
}

TEST(unit_test_rage_engine, RightVector)
{
    const auto right = rage_engine::right_vector({});

    EXPECT_EQ(right, rage_engine::k_abs_right);
}

TEST(unit_test_rage_engine, UpVector)
{
    const auto up = rage_engine::up_vector({});
    EXPECT_EQ(up, rage_engine::k_abs_up);
}

TEST(unit_test_rage_engine, LookAtForward)
{
    const auto angles = rage_engine::CameraTrait::calc_look_at_angle({}, rage_engine::k_abs_forward);

    expect_rage_vector_near(rage_engine::forward_vector(angles), rage_engine::k_abs_forward);
}

TEST(unit_test_rage_engine, LookAtRight)
{
    const auto angles = rage_engine::CameraTrait::calc_look_at_angle({}, rage_engine::k_abs_right);

    expect_rage_vector_near(rage_engine::forward_vector(angles), rage_engine::k_abs_right);
}

TEST(unit_test_rage_engine, LookAtUp)
{
    const auto angles = rage_engine::CameraTrait::calc_look_at_angle({}, rage_engine::k_abs_up);

    expect_rage_vector_near(rage_engine::forward_vector(angles), rage_engine::k_abs_up);
}

TEST(unit_test_rage_engine, ProjectTargetMovedFromCamera)
{
    constexpr auto fov = projection::FieldOfView::from_degrees(60.f);
    const auto cam = rage_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.01f, 1000.f);

    const auto projected = cam.world_to_screen({0, 10.f, 0});

    ASSERT_TRUE(projected.has_value());
    EXPECT_NEAR(projected->x, 640.f, 0.0001f);
    EXPECT_NEAR(projected->y, 360.f, 0.0001f);
}

TEST(unit_test_rage_engine, PredEngineTraitUsesZAsHeight)
{
    projectile_prediction::Projectile<float> projectile;
    projectile.m_origin = {0.f, 0.f, 0.f};
    projectile.m_launch_speed = 10.f;
    projectile.m_gravity_scale = 1.f;

    const auto pos = rage_engine::PredEngineTrait::predict_projectile_position(projectile, 0.f, 0.f, 1.f, 9.81f);

    EXPECT_NEAR(pos.x, 0.f, 0.0001f);
    EXPECT_NEAR(pos.y, 10.f, 0.0001f);
    EXPECT_NEAR(pos.z, -9.81f * 0.5f, 0.0001f);
    EXPECT_NEAR(rage_engine::PredEngineTrait::get_vector_height_coordinate({1.f, 2.f, 3.f}), 3.f, 0.0001f);
}

TEST(unit_test_rage_engine, MeshTraitForwardsRotationMatrix)
{
    const rage_engine::ViewAngles angles{
            rage_engine::PitchAngle::from_degrees(20.f),
            rage_engine::YawAngle::from_degrees(-35.f),
            rage_engine::RollAngle::from_degrees(15.f),
    };

    EXPECT_EQ(rage_engine::MeshTrait::rotation_matrix(angles), rage_engine::rotation_matrix(angles));
}
