//
// Created by vlad on 2/4/24.
//

#pragma once

#include "uml/Vector3.h"
#include <cstdint>
#include "uml/Vector4.h"


namespace uml::color
{
    [[nodiscard]]
    Vector3 Blend(const Vector3& first, const Vector3& second, float ratio);

    class Color : public Vector4
    {
        public:
            Color(float r, float g, float, float b, float a = 1.f);
            Color(const Vector4& vec);
            [[nodiscard]] Color Blend(const Color& other, float ratio) const;
        explicit operator Vector4() {return {x,y,z,w};}
    };
}