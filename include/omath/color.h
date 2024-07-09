//
// Created by vlad on 2/4/24.
//

#pragma once

#include "omath/Vector3.h"
#include <cstdint>
#include "omath/Vector4.h"


namespace omath::color
{
    struct HSV
    {
        float m_hue{};
        float m_saturation{};
        float m_value{};
    };

    [[nodiscard]]
    Vector3 Blend(const Vector3& first, const Vector3& second, float ratio);

    class Color : public Vector4
    {
        public:
            Color(float r, float g, float b, float a);

            [[nodiscard]]
            static Color FromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

            [[nodiscard]]
            static Color FromHSV(float hue, float saturation, float value);

            [[nodiscard]]
            HSV ToHSV() const;

            explicit Color(Vector4 vec);

            [[nodiscard]]
            Color Blend(const Color& other, float ratio) const;

        [[nodiscard]] static Color Red() {return {1.f, 0.f, 0.f, 1.f};}
        [[nodiscard]] static Color Green() {return {0.f, 1.f, 0.f, 1.f};}
        [[nodiscard]] static Color Blue() {return {0.f, 0.f, 1.f, 1.f};}
    };
}