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
static void verify_world_to_radar_uses_engine_world_axes(const Vector3<NumericType>& world_forward,
                                                         const Vector3<NumericType>& world_right)
{
    const Vector3<NumericType> origin{NumericType{10}, NumericType{-20}, NumericType{5}};
    const NumericType world_distance = NumericType{20};
    const NumericType scale = NumericType{0.5};
    const auto radar_distance = static_cast<float>(world_distance * scale);

    CameraType camera{
            origin,           {}, {1280.f, 720.f}, projection::FieldOfView::from_degrees(90.f), NumericType{0.01},
            NumericType{1000}};

    const auto forward_position = origin + world_forward * world_distance;
    const auto forward_radar = algorithm::world_to_radar(camera, forward_position, scale);

    EXPECT_NEAR(forward_radar.x, 0.f, 1e-4f);
    EXPECT_NEAR(forward_radar.y, -radar_distance, 1e-4f);

    const auto right_position = origin + world_right * world_distance;
    const auto right_radar = algorithm::world_to_radar(camera, right_position, scale);

    EXPECT_NEAR(right_radar.x, radar_distance, 1e-4f);
    EXPECT_NEAR(right_radar.y, 0.f, 1e-4f);
}

template<class CameraType, class NumericType>
static void verify_world_to_radar_uses_changed_camera_yaw(const float yaw_degrees,
                                                          const Vector3<NumericType>& world_forward,
                                                          const Vector3<NumericType>& world_right)
{
    const Vector3<NumericType> origin{NumericType{10}, NumericType{-20}, NumericType{5}};
    const NumericType world_distance = NumericType{20};
    const NumericType scale = NumericType{0.5};
    const auto radar_distance = static_cast<float>(world_distance * scale);

    CameraType camera{
            origin,           {}, {1280.f, 720.f}, projection::FieldOfView::from_degrees(90.f), NumericType{0.01},
            NumericType{1000}};

    auto angles = camera.get_view_angles();
    angles.yaw = decltype(angles.yaw)::from_degrees(yaw_degrees);
    camera.set_view_angles(angles);

    const auto forward_position = origin + world_right * world_distance;
    const auto forward_radar = algorithm::world_to_radar(camera, forward_position, scale);

    EXPECT_NEAR(forward_radar.x, 0.f, 1e-4f);
    EXPECT_NEAR(forward_radar.y, -radar_distance, 1e-4f);

    const auto left_position = origin + world_forward * world_distance;
    const auto left_radar = algorithm::world_to_radar(camera, left_position, scale);

    EXPECT_NEAR(left_radar.x, -radar_distance, 1e-4f);
    EXPECT_NEAR(left_radar.y, 0.f, 1e-4f);
}

template<class CameraType, class NumericType>
static void verify_world_to_radar_ignores_camera_pitch(const Vector3<NumericType>& world_forward)
{
    const Vector3<NumericType> origin{NumericType{10}, NumericType{-20}, NumericType{5}};
    const NumericType world_distance = NumericType{20};
    const NumericType scale = NumericType{0.5};
    const auto radar_distance = static_cast<float>(world_distance * scale);

    CameraType camera{
            origin,           {}, {1280.f, 720.f}, projection::FieldOfView::from_degrees(90.f), NumericType{0.01},
            NumericType{1000}};

    auto angles = camera.get_view_angles();
    angles.pitch = decltype(angles.pitch)::from_degrees(45.f);
    camera.set_view_angles(angles);

    const auto forward_position = origin + world_forward * world_distance;
    const auto forward_radar = algorithm::world_to_radar(camera, forward_position, scale);

    EXPECT_NEAR(forward_radar.x, 0.f, 1e-4f);
    EXPECT_NEAR(forward_radar.y, -radar_distance, 1e-4f);
}

TEST(WorldToRadarTests, SourceEngineCamera)
{
    verify_world_to_radar_uses_engine_world_axes<source_engine::Camera, float>(source_engine::k_abs_forward,
                                                                               source_engine::k_abs_right);
    verify_world_to_radar_uses_changed_camera_yaw<source_engine::Camera, float>(-90.f, source_engine::k_abs_forward,
                                                                                source_engine::k_abs_right);
    verify_world_to_radar_ignores_camera_pitch<source_engine::Camera, float>(source_engine::k_abs_forward);
}

TEST(WorldToRadarTests, IWEngineCamera)
{
    verify_world_to_radar_uses_engine_world_axes<iw_engine::Camera, float>(iw_engine::k_abs_forward,
                                                                           iw_engine::k_abs_right);
    verify_world_to_radar_uses_changed_camera_yaw<iw_engine::Camera, float>(-90.f, iw_engine::k_abs_forward,
                                                                            iw_engine::k_abs_right);
    verify_world_to_radar_ignores_camera_pitch<iw_engine::Camera, float>(iw_engine::k_abs_forward);
}

TEST(WorldToRadarTests, FrostbiteEngineCamera)
{
    verify_world_to_radar_uses_engine_world_axes<frostbite_engine::Camera, float>(frostbite_engine::k_abs_forward,
                                                                                  frostbite_engine::k_abs_right);
    verify_world_to_radar_uses_changed_camera_yaw<frostbite_engine::Camera, float>(
            90.f, frostbite_engine::k_abs_forward, frostbite_engine::k_abs_right);
    verify_world_to_radar_ignores_camera_pitch<frostbite_engine::Camera, float>(frostbite_engine::k_abs_forward);
}

TEST(WorldToRadarTests, OpenGLEngineCamera)
{
    verify_world_to_radar_uses_engine_world_axes<opengl_engine::Camera, float>(opengl_engine::k_abs_forward,
                                                                               opengl_engine::k_abs_right);
    verify_world_to_radar_uses_changed_camera_yaw<opengl_engine::Camera, float>(-90.f, opengl_engine::k_abs_forward,
                                                                                opengl_engine::k_abs_right);
    verify_world_to_radar_ignores_camera_pitch<opengl_engine::Camera, float>(opengl_engine::k_abs_forward);
}

TEST(WorldToRadarTests, UnityEngineCamera)
{
    verify_world_to_radar_uses_engine_world_axes<unity_engine::Camera, float>(unity_engine::k_abs_forward,
                                                                              unity_engine::k_abs_right);
    verify_world_to_radar_uses_changed_camera_yaw<unity_engine::Camera, float>(90.f, unity_engine::k_abs_forward,
                                                                               unity_engine::k_abs_right);
    verify_world_to_radar_ignores_camera_pitch<unity_engine::Camera, float>(unity_engine::k_abs_forward);
}

TEST(WorldToRadarTests, CryEngineCamera)
{
    verify_world_to_radar_uses_engine_world_axes<cry_engine::Camera, float>(cry_engine::k_abs_forward,
                                                                            cry_engine::k_abs_right);
    verify_world_to_radar_uses_changed_camera_yaw<cry_engine::Camera, float>(-90.f, cry_engine::k_abs_forward,
                                                                             cry_engine::k_abs_right);
    verify_world_to_radar_ignores_camera_pitch<cry_engine::Camera, float>(cry_engine::k_abs_forward);
}

TEST(WorldToRadarTests, RageEngineCamera)
{
    verify_world_to_radar_uses_engine_world_axes<rage_engine::Camera, float>(rage_engine::k_abs_forward,
                                                                             rage_engine::k_abs_right);
    verify_world_to_radar_uses_changed_camera_yaw<rage_engine::Camera, float>(-90.f, rage_engine::k_abs_forward,
                                                                              rage_engine::k_abs_right);
    verify_world_to_radar_ignores_camera_pitch<rage_engine::Camera, float>(rage_engine::k_abs_forward);
}

TEST(WorldToRadarTests, UnrealEngineCamera)
{
    verify_world_to_radar_uses_engine_world_axes<unreal_engine::Camera, double>(unreal_engine::k_abs_forward,
                                                                                unreal_engine::k_abs_right);
    verify_world_to_radar_uses_changed_camera_yaw<unreal_engine::Camera, double>(90.f, unreal_engine::k_abs_forward,
                                                                                 unreal_engine::k_abs_right);
    verify_world_to_radar_ignores_camera_pitch<unreal_engine::Camera, double>(unreal_engine::k_abs_forward);
}
