//
// Created by Vlad on 8/28/2025.
//
module omath.primitives_3d.plane;

import omath.collision.triangle;
import omath.linear_algebra.vector3;

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