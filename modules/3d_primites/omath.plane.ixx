//
// Created by Vlad on 9/1/2025.
//
module;
#include <array>

export module omath.plane;
export import omath.triangle;

export namespace omath::primitives
{
    [[nodiscard]]
    std::array<Triangle<>, 2> create_plane(const Vector3<float>& vertex_a, const Vector3<float>& vertex_b,
                                           const Vector3<float>& direction, const float size) noexcept
    {
        const auto second_vertex_a = vertex_a + direction * size;
        return std::array
        {
            Triangle{second_vertex_a, vertex_a, vertex_b},
            Triangle{second_vertex_a, vertex_b + direction * size, vertex_b}
        };
    }
}
