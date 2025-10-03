//
// Created by Vlad on 8/28/2025.
//

module;
#include <array>

export module omath.primitives_3d.plane;

export import omath.collision.triangle;
export import omath.linear_algebra.vector3;

namespace omath::primitives
{
    [[nodiscard]]
    std::array<Triangle<Vector3<float>>, 2> create_plane(const Vector3<float>& vertex_a, const Vector3<float>& vertex_b,
                                                         const Vector3<float>& direction, float size) noexcept;
}
