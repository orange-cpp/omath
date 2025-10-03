//
// Created by Vlad on 4/18/2025.
//
module;
#include <array>

export module omath.primitives_3d.box;
export import omath.collision.triangle;
export import omath.linear_algebra.vector3;

namespace omath::primitives
{
    [[nodiscard]]
    std::array<Triangle<Vector3<float>>, 12> create_box(const Vector3<float>& top, const Vector3<float>& bottom,
                                             const Vector3<float>& dir_forward, const Vector3<float>& dir_right,
                                             float ratio = 4.f) noexcept;
}
