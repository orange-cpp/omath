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

#ifdef OMATH_USE_GCEM
namespace
{
    constexpr bool close_to(const float actual, const float expected, const float epsilon)
    {
        const float diff = actual - expected;
        return (diff < 0.0f ? -diff : diff) <= epsilon;
    }

    constexpr bool vec_close_to(const Vector3<float>& actual, const Vector3<float>& expected, const float epsilon)
    {
        return close_to(actual.x, expected.x, epsilon) && close_to(actual.y, expected.y, epsilon)
               && close_to(actual.z, expected.z, epsilon);
    }
} // namespace

static_assert(
        []
        {
            constexpr omath::cry_engine::ViewAngles angles{};
            constexpr auto forward = omath::cry_engine::forward_vector(angles);
            constexpr auto right = omath::cry_engine::right_vector(angles);
            constexpr auto up = omath::cry_engine::up_vector(angles);
            constexpr auto rotation = omath::cry_engine::rotation_matrix(angles);

            return vec_close_to(forward, omath::cry_engine::k_abs_forward, 1e-5f)
                   && vec_close_to(right, omath::cry_engine::k_abs_right, 1e-5f)
                   && vec_close_to(up, omath::cry_engine::k_abs_up, 1e-5f) && close_to(rotation.at(0, 0), 1.0f, 1e-5f)
                   && close_to(rotation.at(1, 1), 1.0f, 1e-5f) && close_to(rotation.at(2, 2), 1.0f, 1e-5f)
                   && close_to(rotation.at(3, 3), 1.0f, 1e-5f);
        }(),
        "CryEngine basis vectors should be constexpr with gcem");

static_assert(
        []
        {
            constexpr auto view = omath::cry_engine::calc_view_matrix({}, {});
            return close_to(view.at(0, 0), 1.0f, 1e-5f) && close_to(view.at(1, 2), 1.0f, 1e-5f)
                   && close_to(view.at(2, 1), 1.0f, 1e-5f) && close_to(view.at(3, 3), 1.0f, 1e-5f);
        }(),
        "CryEngine view matrix should be constexpr with gcem");

static_assert(
        []
        {
            constexpr auto transform = mat_translation<float, MatStoreType::ROW_MAJOR>({1.0f, 2.0f, 3.0f})
                                       * mat_scale<float, MatStoreType::ROW_MAJOR>({2.0f, 3.0f, 4.0f});
            constexpr auto origin = omath::cry_engine::extract_origin(transform);
            constexpr auto scale = omath::cry_engine::extract_scale(transform);

            return vec_close_to(origin, {1.0f, 2.0f, 3.0f}, 1e-5f) && vec_close_to(scale, {2.0f, 3.0f, 4.0f}, 1e-5f);
        }(),
        "CryEngine transform extraction should be constexpr with gcem");

static_assert(
        []
        {
            constexpr auto projection = omath::cry_engine::calc_perspective_projection_matrix(
                    90.0f, 1.0f, 1.0f, 11.0f, NDCDepthRange::ZERO_TO_ONE);
            return close_to(projection.at(0, 0), 1.0f, 1e-5f) && close_to(projection.at(1, 1), 1.0f, 1e-5f)
                   && close_to(projection.at(2, 2), 1.1f, 1e-5f) && close_to(projection.at(2, 3), -1.1f, 1e-5f)
                   && close_to(projection.at(3, 2), 1.0f, 1e-5f);
        }(),
        "CryEngine projection matrix should be constexpr with gcem");

static_assert(
        []
        {
            constexpr auto angles =
                    omath::cry_engine::CameraTrait::calc_look_at_angle({}, omath::cry_engine::k_abs_forward);
            constexpr auto view = omath::cry_engine::CameraTrait::calc_view_matrix(angles, {});
            constexpr auto projection = omath::cry_engine::CameraTrait::calc_projection_matrix(
                    projection::FieldOfView::from_degrees(90.0f), {1.0f, 1.0f}, 1.0f, 11.0f,
                    NDCDepthRange::ZERO_TO_ONE);

            return close_to(angles.pitch.as_degrees(), 0.0f, 1e-5f) && close_to(angles.yaw.as_degrees(), 0.0f, 1e-5f)
                   && close_to(angles.roll.as_degrees(), 0.0f, 1e-5f) && close_to(view.at(0, 0), 1.0f, 1e-5f)
                   && close_to(view.at(2, 1), 1.0f, 1e-5f) && close_to(projection.at(0, 0), 1.0f, 1e-5f)
                   && close_to(projection.at(1, 1), 1.0f, 1e-5f) && close_to(projection.at(2, 2), 1.1f, 1e-5f)
                   && close_to(projection.at(2, 3), -1.1f, 1e-5f) && close_to(projection.at(3, 2), 1.0f, 1e-5f);
        }(),
        "CryEngine CameraTrait should be constexpr with gcem");

static_assert(omath::cry_engine::units_to_centimeters(100.0f) == 1.0f);
static_assert(omath::cry_engine::units_to_meters(2.0f) == 2.0f);
static_assert(omath::cry_engine::units_to_kilometers(2000.0f) == 2.0f);
static_assert(omath::cry_engine::centimeters_to_units(1.0f) == 100.0f);
static_assert(omath::cry_engine::meters_to_units(2.0f) == 2.0f);
static_assert(omath::cry_engine::kilometers_to_units(2.0f) == 2000.0f);
#endif

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
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, cry_engine::k_abs_up);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), cry_engine::k_abs_up.as_array()))
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

TEST(unit_test_cry_engine, ViewAnglesAsVector3Zero)
{
    const omath::cry_engine::ViewAngles angles{};
    const auto vec = angles.as_vector3();

    EXPECT_FLOAT_EQ(vec.x, 0.f);
    EXPECT_FLOAT_EQ(vec.y, 0.f);
    EXPECT_FLOAT_EQ(vec.z, 0.f);
}

TEST(unit_test_cry_engine, ViewAnglesAsVector3Values)
{
    const omath::cry_engine::ViewAngles angles{omath::cry_engine::PitchAngle::from_degrees(45.f),
                                               omath::cry_engine::YawAngle::from_degrees(-90.f),
                                               omath::cry_engine::RollAngle::from_degrees(30.f)};
    const auto vec = angles.as_vector3();

    EXPECT_FLOAT_EQ(vec.x, 45.f);
    EXPECT_FLOAT_EQ(vec.y, -90.f);
    EXPECT_FLOAT_EQ(vec.z, 30.f);
}

TEST(unit_test_cry_engine, ViewAnglesAsVector3ClampedPitch)
{
    // Pitch is clamped to [-90, 90]
    const omath::cry_engine::ViewAngles angles{omath::cry_engine::PitchAngle::from_degrees(120.f),
                                               omath::cry_engine::YawAngle::from_degrees(0.f),
                                               omath::cry_engine::RollAngle::from_degrees(0.f)};
    const auto vec = angles.as_vector3();

    EXPECT_FLOAT_EQ(vec.x, 90.f);
}

TEST(unit_test_cry_engine, ViewAnglesAsVector3NormalizedYaw)
{
    // Yaw is normalized to [-180, 180], 270 wraps to -90
    const omath::cry_engine::ViewAngles angles{omath::cry_engine::PitchAngle::from_degrees(0.f),
                                               omath::cry_engine::YawAngle::from_degrees(270.f),
                                               omath::cry_engine::RollAngle::from_degrees(0.f)};
    const auto vec = angles.as_vector3();

    EXPECT_NEAR(vec.y, -90.f, 0.01f);
}
