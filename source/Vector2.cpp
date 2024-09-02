//
// Created by Vlad on 02.09.2024.
//
#include "omath/Vector2.h"
#include <cmath>


namespace omath
{

    Vector2 Vector2::Normalized() const
    {
        const float len = Length();

        if (len > 0.f)
            return {x / len, y / len};

        return {0.f, 0.f};
    }

    float Vector2::Length() const
    {
        return std::sqrt(x * x + y * y);
    }
}