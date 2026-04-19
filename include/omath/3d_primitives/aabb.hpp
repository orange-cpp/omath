//
// Created by Vladislav on 24.03.2026.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"

namespace omath::primitives
{
    template<class Type>
    struct Aabb final
    {
        Vector3<Type> min;
        Vector3<Type> max;

        [[nodiscard]]
        constexpr Vector3<Type> center() const noexcept
        {
            return (min + max) / static_cast<Type>(2);
        }

        [[nodiscard]]
        constexpr Vector3<Type> extents() const noexcept
        {
            return (max - min) / static_cast<Type>(2);
        }
        [[nodiscard]]
        constexpr bool is_collide(const Aabb& other) const noexcept
        {
            return min.x <= other.max.x && max.x >= other.min.x &&
                min.y <= other.max.y && max.y >= other.min.y &&min.z <= other.max.z && max.z >= other.min.z;
        }
    };
} // namespace omath::primitives
