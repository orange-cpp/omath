//
// Created by Vladislav on 19.02.2026.
//
#include <gtest/gtest.h>
#include <omath/engines/cry_engine/camera.hpp>
#include <omath/engines/cry_engine/constants.hpp>
#include <omath/engines/cry_engine/formulas.hpp>
#include <random>
#include <ranges>

using namespace omath;
TEST(unit_test_cry_engine, look_at_forward)
{
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, cry_engine::k_abs_forward);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), cry_engine::k_abs_forward.as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}
TEST(unit_test_cry_engine, look_at_right)
{
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, cry_engine::k_abs_right);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), cry_engine::k_abs_right.as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}
TEST(unit_test_cry_engine, look_at_up)
{
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, cry_engine::k_abs_right);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), cry_engine::k_abs_right.as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}

TEST(unit_test_cry_engine, look_at_back)
{
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, -cry_engine::k_abs_forward);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), (-cry_engine::k_abs_forward).as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}
TEST(unit_test_cry_engine, look_at_left)
{
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, -cry_engine::k_abs_right);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), (-cry_engine::k_abs_right).as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}
TEST(unit_test_cry_engine, look_at_down)
{
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, -cry_engine::k_abs_up);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), (-cry_engine::k_abs_up).as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}

TEST(unit_test_cry_engine, RightVector)
{
    const auto right = omath::cry_engine::right_vector({});

    EXPECT_EQ(right, omath::cry_engine::k_abs_right);
}

TEST(unit_test_cry_engine, UpVector)
{
    const auto up = omath::cry_engine::up_vector({});
    EXPECT_EQ(up, omath::cry_engine::k_abs_up);
}

TEST(unit_test_cry_engine, ProjectTargetMovedFromCamera)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);
    const auto cam = omath::cry_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.01f, 1000.f);

    for (float distance = 0.02f; distance < 100.f; distance += 0.01f)
    {
        const auto projected = cam.world_to_screen({0, distance, 0});

        EXPECT_TRUE(projected.has_value());

        if (!projected.has_value())
            continue;

        EXPECT_NEAR(projected->x, 640, 0.00001f);
        EXPECT_NEAR(projected->y, 360, 0.00001f);
    }
}

TEST(unit_test_cry_engine, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::cry_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 90.f);
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}

TEST(unit_test_cry_engine, CameraSetAndGetOrigin)
{
    auto cam = omath::cry_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_origin(), omath::Vector3<float>{});
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}
TEST(unit_test_cry_engine, loook_at_random_all_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::cry_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

    std::size_t failed_points = 0;

    for (int i = 0; i < 1000; i++)
    {
        const auto position_to_look = omath::Vector3<float>{dist(gen), dist(gen), dist(gen)};

        if (cam.get_origin().distance_to(position_to_look) < 10)
            continue;

        cam.look_at(position_to_look);

        auto projected_pos = cam.world_to_view_port(position_to_look);

        EXPECT_TRUE(projected_pos.has_value());

        if (!projected_pos)
            continue;

        if (std::abs(projected_pos->x - 0.f) >= 0.0001f || std::abs(projected_pos->y - 0.f) >= 0.0001f)
            failed_points++;
    }
    EXPECT_LE(failed_points, 100);
}

TEST(unit_test_cry_engine, loook_at_random_x_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::cry_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

    std::size_t failed_points = 0;
    for (int i = 0; i < 1000; i++)
    {
        const auto position_to_look = omath::Vector3<float>{dist(gen), 0.f, 0.f};
        if (cam.get_origin().distance_to(position_to_look) < 10)
            continue;

        cam.look_at(position_to_look);

        auto projected_pos = cam.world_to_view_port(position_to_look);

        EXPECT_TRUE(projected_pos.has_value());

        if (!projected_pos)
            continue;

        if (std::abs(projected_pos->x - 0.f) >= 0.001f || std::abs(projected_pos->y - 0.f) >= 0.001f)
            failed_points++;
    }
    EXPECT_LE(failed_points, 100);
}

TEST(unit_test_cry_engine, loook_at_random_y_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::cry_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

    std::size_t failed_points = 0;
    for (int i = 0; i < 1000; i++)
    {
        const auto position_to_look = omath::Vector3<float>{0.f, dist(gen), 0.f};
        if (cam.get_origin().distance_to(position_to_look) < 10)
            continue;

        cam.look_at(position_to_look);

        auto projected_pos = cam.world_to_view_port(position_to_look);

        EXPECT_TRUE(projected_pos.has_value());

        if (!projected_pos)
            continue;

        if (std::abs(projected_pos->x - 0.f) >= 0.01f || std::abs(projected_pos->y - 0.f) >= 0.01f)
            failed_points++;
    }
    EXPECT_LE(failed_points, 100);
}

TEST(unit_test_cry_engine, loook_at_random_z_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::cry_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

    std::size_t failed_points = 0;
    for (int i = 0; i < 1000; i++)
    {
        const auto position_to_look = omath::Vector3<float>{0.f, 0.f, dist(gen)};
        if (cam.get_origin().distance_to(position_to_look) < 10)
            continue;

        cam.look_at(position_to_look);

        auto projected_pos = cam.world_to_view_port(position_to_look);

        EXPECT_TRUE(projected_pos.has_value());

        if (!projected_pos)
            continue;

        if (std::abs(projected_pos->x - 0.f) >= 0.01f || std::abs(projected_pos->y - 0.f) >= 0.01f)
            failed_points++;
    }
    EXPECT_LE(failed_points, 100);
}