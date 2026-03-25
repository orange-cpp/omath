//
// Created by Vlad on 3/25/2025.
//
#pragma once

#include "omath/linear_algebra/vector3.hpp"

namespace omath
{
    template<class Vector = Vector3<float>>
    class AABB final
    {
    public:
        using VectorType = Vector;

        VectorType min;
        VectorType max;

        constexpr AABB(const VectorType& min, const VectorType& max) noexcept : min(min), max(max) {}

        [[nodiscard]]
        constexpr VectorType center() const noexcept
        {
            return (min + max) / static_cast<typename VectorType::ContainedType>(2);
        }

        [[nodiscard]]
        constexpr VectorType extents() const noexcept
        {
            return (max - min) / static_cast<typename VectorType::ContainedType>(2);
        }
    };
} // namespace omath
