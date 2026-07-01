//
// Created by orange on 7/1/2026.
//
#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include <type_traits>

namespace omath::algorithm
{
    template<class Camera, class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    Vector2<float> world_to_radar(const Camera& camera, const Vector3<FloatingType>& position, const FloatingType scale)
    {
        auto look_at_angles = camera.calc_look_at_angles(position);
        auto current_angles = camera.get_view_angles();

        auto yaw = look_at_angles.yaw - current_angles.yaw + decltype(current_angles.yaw)::from_degrees(180);
        auto right_yaw = camera.calc_look_at_angles(camera.get_origin() + camera.get_abs_right()).yaw
                         - current_angles.yaw + decltype(current_angles.yaw)::from_degrees(180);
        const auto right_scale = right_yaw.sin() < 0 ? -1.f : 1.f;

        return omath::Vector2<float>(static_cast<float>(yaw.sin()) * right_scale, yaw.cos())
               * (camera.get_origin().distance_to(position) * scale);
    }
} // namespace omath::algorithm
