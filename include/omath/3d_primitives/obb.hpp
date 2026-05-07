//
// Created by Vladislav on 07.05.2026.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include <array>
#include <type_traits>

namespace omath::primitives
{
    // Oriented bounding box: a rectangular cuboid defined by a center, three
    // orthonormal local axes, and the half-size along each of those axes.
    template<class Type>
    requires std::is_floating_point_v<Type>
    struct Obb final
    {
        Vector3<Type> center;
        Vector3<Type> axis_x;
        Vector3<Type> axis_y;
        Vector3<Type> axis_z;
        Vector3<Type> half_extents;

        [[nodiscard]]
        constexpr std::array<Vector3<Type>, 8> vertices() const noexcept
        {
            const auto ex = axis_x * half_extents.x;
            const auto ey = axis_y * half_extents.y;
            const auto ez = axis_z * half_extents.z;

            return {
                    center - ex - ey - ez, center + ex - ey - ez, center - ex + ey - ez, center + ex + ey - ez,
                    center - ex - ey + ez, center + ex - ey + ez, center - ex + ey + ez, center + ex + ey + ez,
            };
        }
    };
} // namespace omath::primitives
