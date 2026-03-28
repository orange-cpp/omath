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
    };
} // namespace omath::primitives
