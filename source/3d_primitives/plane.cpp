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
        const auto second_vertex_a = vertex_a + direction * size;
        return std::array
        {
            Triangle{second_vertex_a, vertex_a, vertex_b},
            Triangle{second_vertex_a, vertex_b + direction * size, vertex_b}
        };
    }
} // namespace omath::primitives