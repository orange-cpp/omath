//
// Created by Vlad on 4/18/2025.
//

#pragma once
#include "omath/linear_algebra/triangle.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <array>

namespace omath::primitives
{
    [[nodiscard]]
    std::array<Triangle<Vector3<float>>, 12> create_box(const Vector3<float>& top, const Vector3<float>& bottom,
                                             const Vector3<float>& dir_forward, const Vector3<float>& dir_right,
                                             float ratio = 4.f) noexcept;
}
