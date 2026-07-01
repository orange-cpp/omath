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
        const auto forward = static_cast<Vector3<FloatingType>>(camera.get_abs_forward());
        const auto right = static_cast<Vector3<FloatingType>>(camera.get_abs_right());
        const auto direction = position - origin;

        return {static_cast<float>(direction.dot(right) * scale), static_cast<float>(-direction.dot(forward) * scale),
                0.f};
    }
} // namespace omath::algorithm
