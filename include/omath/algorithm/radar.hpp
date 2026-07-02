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

        const auto look_at_angles = camera.calc_look_at_angles(position);
        const auto current_angles = camera.get_view_angles();

        static const auto sign = [&camera, &current_angles]
        {
            auto right_yaw = current_angles.yaw
                           - camera.calc_look_at_angles(camera.get_origin() + camera.get_abs_right()).yaw
                           - decltype(current_angles.yaw)::from_degrees(90);
            return right_yaw.cos() < 0 ? -1.f : 1.f;
        }();

        const auto yaw = current_angles.yaw - look_at_angles.yaw - decltype(current_angles.yaw)::from_degrees(90);

        return omath::Vector2<float>(static_cast<float>(yaw.cos()) * sign, static_cast<float>(yaw.sin()))
               * (camera.get_origin().distance_to(position) * scale);
    }
} // namespace omath::algorithm
