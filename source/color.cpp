//
// Created by vlad on 2/4/24.
//

#include "uml/color.h"

namespace uml::color
{
    Vector3 Blend(const uml::Vector3 &first, const uml::Vector3 &second, float ratio)
    {
        return first * (1.f - ratio) + second * ratio;
    }
}