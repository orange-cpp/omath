//
// Created by Vladislav on 24.03.2026.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"

namespace omath::primitives
{
    enum class UpAxis { X, Y, Z };

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

        template<UpAxis Up = UpAxis::Y>
        [[nodiscard]]
        constexpr Vector3<Type> top() const noexcept
        {
            const auto aabb_center = center();
            if constexpr (Up == UpAxis::Z)
                return {aabb_center.x, aabb_center.y, max.z};
            else if constexpr (Up == UpAxis::X)
                return {max.x, aabb_center.y, aabb_center.z};
            else if constexpr (Up == UpAxis::Y)
                return {aabb_center.x, max.y, aabb_center.z};
            else
                std::unreachable();
        }

        template<UpAxis Up = UpAxis::Y>
        [[nodiscard]]
        constexpr Vector3<Type> bottom() const noexcept
        {
            const auto aabb_center = center();
            if constexpr (Up == UpAxis::Z)
                return {aabb_center.x, aabb_center.y, min.z};
            else if constexpr (Up == UpAxis::X)
                return {min.x, aabb_center.y, aabb_center.z};
            else if constexpr (Up == UpAxis::Y)
                return {aabb_center.x, min.y, aabb_center.z};
            else
                std::unreachable();
        }

        [[nodiscard]]
        constexpr bool is_collide(const Aabb& other) const noexcept
        {
            return min.x <= other.max.x && max.x >= other.min.x &&
                min.y <= other.max.y && max.y >= other.min.y &&min.z <= other.max.z && max.z >= other.min.z;
        }
    };
} // namespace omath::primitives
