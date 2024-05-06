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
        return Color( (*this * (1.f - ratio)) + (other * ratio) );
    }

    Color::Color(float r, float g, float b, float a) : Vector4(r,g,b,a)
    {

    }

    Color::Color(const Vector4 &vec) : Vector4(vec)
    {

    }

    Color Color::FromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
    {

        return Color{Vector4(r, g, b, a) / 255.f};

    }
}