#include <gtest/gtest.h>
#include <omath/algorithm/radar.hpp>
#include <omath/engines/cry_engine/camera.hpp>
#include <omath/engines/frostbite_engine/camera.hpp>
#include <omath/engines/iw_engine/camera.hpp>
#include <omath/engines/opengl_engine/camera.hpp>
#include <omath/engines/rage_engine/camera.hpp>
#include <omath/engines/source_engine/camera.hpp>
#include <omath/engines/unity_engine/camera.hpp>
#include <omath/engines/unreal_engine/camera.hpp>

using namespace omath;

template<class CameraType, class NumericType>
static void verify_world_to_radar_uses_camera_axes()
{
    const Vector3<NumericType> origin{NumericType{10}, NumericType{-20}, NumericType{5}};
    const NumericType world_distance = NumericType{20};
    const NumericType scale = NumericType{0.5};
    const auto radar_distance = static_cast<float>(world_distance * scale);

    CameraType camera{
            origin,           {}, {1280.f, 720.f}, projection::FieldOfView::from_degrees(90.f), NumericType{0.01},
            NumericType{1000}};

    const auto forward_position = origin + camera.get_abs_forward() * world_distance;
    const auto forward_radar = algorithm::world_to_radar(camera, forward_position, scale);

    EXPECT_NEAR(forward_radar.x, 0.f, 1e-4f);
    EXPECT_NEAR(forward_radar.y, -radar_distance, 1e-4f);
    EXPECT_NEAR(forward_radar.z, 0.f, 1e-6f);

    const auto right_position = origin + camera.get_abs_right() * world_distance;
    const auto right_radar = algorithm::world_to_radar(camera, right_position, scale);

    EXPECT_NEAR(right_radar.x, radar_distance, 1e-4f);
    EXPECT_NEAR(right_radar.y, 0.f, 1e-4f);
    EXPECT_NEAR(right_radar.z, 0.f, 1e-6f);
}

TEST(WorldToRadarTests, SourceEngineCamera)
{
    verify_world_to_radar_uses_camera_axes<source_engine::Camera, float>();
}

TEST(WorldToRadarTests, IWEngineCamera)
{
    verify_world_to_radar_uses_camera_axes<iw_engine::Camera, float>();
}

TEST(WorldToRadarTests, FrostbiteEngineCamera)
{
    verify_world_to_radar_uses_camera_axes<frostbite_engine::Camera, float>();
}

TEST(WorldToRadarTests, OpenGLEngineCamera)
{
    verify_world_to_radar_uses_camera_axes<opengl_engine::Camera, float>();
}

TEST(WorldToRadarTests, UnityEngineCamera)
{
    verify_world_to_radar_uses_camera_axes<unity_engine::Camera, float>();
}

TEST(WorldToRadarTests, CryEngineCamera)
{
    verify_world_to_radar_uses_camera_axes<cry_engine::Camera, float>();
}

TEST(WorldToRadarTests, RageEngineCamera)
{
    verify_world_to_radar_uses_camera_axes<rage_engine::Camera, float>();
}

TEST(WorldToRadarTests, UnrealEngineCamera)
{
    verify_world_to_radar_uses_camera_axes<unreal_engine::Camera, double>();
}
