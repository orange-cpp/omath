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
    Vector3<float> world_to_radar(const Camera& camera, const Vector3<FloatingType>& position, const FloatingType scale)
    {
        const auto origin = static_cast<Vector3<FloatingType>>(camera.get_origin());
        auto current_angles = camera.get_view_angles();
        auto look_at_angles = camera.calc_look_at_angles(position);
        auto yaw = look_at_angles.yaw - current_angles.yaw;

        const auto right = origin + static_cast<Vector3<FloatingType>>(camera.get_abs_right());
        auto right_yaw = camera.calc_look_at_angles(right).yaw - current_angles.yaw;
        const auto right_sign = right_yaw.sin() < FloatingType{0} ? FloatingType{-1} : FloatingType{1};
        const auto radar_distance = position.distance_to(origin) * scale;

        return {static_cast<float>(yaw.sin() * right_sign * radar_distance),
                static_cast<float>(-yaw.cos() * radar_distance), 0.f};
    }
} // namespace omath::algorithm
