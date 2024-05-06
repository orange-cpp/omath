//
// Created by vlad on 2/4/24.
//

#pragma once
#include "uml/Vector3.h"
#include <cstdint>

namespace uml::color
{
    [[nodiscard]]
    Vector3 Blend(const Vector3& first, const Vector3& second, float ratio);
}