//
// Created by vlad on 2/4/24.
//

#include "uml/color.h"
#include <algorithm>

namespace uml::color
{
    Vector3 Blend(const Vector3 &first, const Vector3 &second, float ratio)
    {
        return first * (1.f - std::clamp(ratio, 0.f, 1.f)) + second * ratio;
    }
    
    Color Color::Blend(const Color &other, float ratio) const
    {

    }
}