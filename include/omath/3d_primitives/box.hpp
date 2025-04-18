//
// Created by Vlad on 4/18/2025.
//

#pragma once
#include <array>
#include "omath/triangle.hpp"
#include "omath/vector3.hpp"


namespace omath::primitives
{
    [[nodiscard]]
    std::array<Triangle<Vector3<float>>, 12> CreateBox(const Vector3<float>& top, const Vector3<float>& bottom,
                                             const Vector3<float>& dirForward, const Vector3<float>& dirRight,
                                             float ratio = 4.f);
}
