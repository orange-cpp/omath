//
// Created by Vlad on 8/28/2025.
//
#include "omath/3d_primitives/plane.hpp"

namespace omath::primitives
{
    std::array<Triangle<Vector3<float>>, 2> create_plane(const Vector3<float>& vertex_a,
                                                          const Vector3<float>& vertex_b,
                                                          const Vector3<float>& direction, const float size) noexcept
    {
        return std::array
        {
            Triangle{vertex_a + direction * size, vertex_a, vertex_b},
            Triangle{vertex_b + direction * size, vertex_b, vertex_a}
        };
    }
} // namespace omath::primitives