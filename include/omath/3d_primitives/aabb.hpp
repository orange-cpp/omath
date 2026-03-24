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
    };
} // namespace omath::primitives
