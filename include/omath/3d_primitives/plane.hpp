//
// Created by Vlad on 8/28/2025.
//

#pragma once
#include "omath/linear_algebra/triangle.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <array>

namespace omath::primitives
{
    [[nodiscard]]
    std::array<Triangle<Vector3<float>>, 2> create_plane(const Vector3<float>& vertex_a,
                                                          const Vector3<float>& vertex_b,
                                                          const Vector3<float>& direction, float size) noexcept;
}
