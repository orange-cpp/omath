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
            Color(float r, float g, float b, float a);
            static Color FromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
            explicit Color(const Vector4& vec);
            [[nodiscard]] Color Blend(const Color& other, float ratio) const;

        [[nodiscard]] static Color Red() {return {1.f, 0.f, 0.f, 1.f};}
        [[nodiscard]] static Color Green() {return {0.f, 1.f, 0.f, 1.f};}
        [[nodiscard]] static Color Blue() {return {0.f, 0.f, 1.f, 1.f};}
    };
}