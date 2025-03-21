//
// Created by vlad on 2/4/24.
//

#pragma once

#include <cstdint>
#include "omath/vector3.hpp"
#include "omath/vector4.hpp"

namespace omath
{
    struct HSV
    {
        float m_hue{};
        float m_saturation{};
        float m_value{};
    };


    class Color final : public Vector4<float>
    {
        public:
            constexpr Color(const float r, const float g, const float b, const float a) : Vector4(r,g,b,a)
            {
                Clamp(0.f, 1.f);
            }

            constexpr explicit Color() : Vector4()
            {

            }
            [[nodiscard]]
            constexpr static Color FromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
            {
                return Color{Vector4(r, g, b, a) / 255.f};
            }

            [[nodiscard]]
            constexpr static Color FromHSV(float hue, float saturation, float value)
            {
                float r{}, g{}, b{};

                hue = std::clamp(hue, 0.f, 1.f);

                const int i = static_cast<int>(hue * 6.f);
                const float f = hue * 6 - i;
                const float p = value * (1 - saturation);
                const float q = value * (1 - f * saturation);
                const float t = value * (1 - (1 - f) * saturation);

                switch (i % 6)
                {
                    case 0: r = value, g = t, b = p; break;
                    case 1: r = q, g = value, b = p; break;
                    case 2: r = p, g = value, b = t; break;
                    case 3: r = p, g = q, b = value; break;
                    case 4: r = t, g = p, b = value; break;
                    case 5: r = value, g = p, b = q; break;

                    default: return {0.f, 0.f, 0.f, 0.f};
                }

                return {r, g, b, 1.f};
            }

            [[nodiscard]]
            constexpr HSV ToHSV() const
            {
                HSV hsvData;

                const float& red = x;
                const float& green = y;
                const float& blue = z;

                const float max = std::max({red, green, blue});
                const float min = std::min({red, green, blue});
                const float delta = max - min;


                if (delta == 0.f)
                    hsvData.m_hue = 0.f;

                else if (max == red)
                    hsvData.m_hue = 60.f * (std::fmodf(((green - blue) / delta), 6.f));
                else if (max == green)
                    hsvData.m_hue = 60.f * (((blue - red) / delta) + 2.f);
                else if (max == blue)
                    hsvData.m_hue = 60.f * (((red - green) / delta) + 4.f);

                if (hsvData.m_hue < 0.f)
                    hsvData.m_hue += 360.f;

                hsvData.m_hue /= 360.f;
                hsvData.m_saturation = max == 0.f ? 0.f : delta / max;
                hsvData.m_value = max;

                return hsvData;
            }

            constexpr explicit Color(const Vector4& vec) : Vector4(vec)
            {
                Clamp(0.f, 1.f);
            }

            [[nodiscard]]
            constexpr Color Blend(const Color& other, float ratio) const
            {
                return Color( (*this * (1.f - ratio)) + (other * ratio) );
            }

        [[nodiscard]] static constexpr Color Red() {return {1.f, 0.f, 0.f, 1.f};}
        [[nodiscard]] static constexpr Color Green() {return {0.f, 1.f, 0.f, 1.f};}
        [[nodiscard]] static constexpr Color Blue() {return {0.f, 0.f, 1.f, 1.f};}
    };

    [[nodiscard]]
    constexpr Color Blend(const Color& first, const Color& second, float ratio)
    {
        return Color{first * (1.f - std::clamp(ratio, 0.f, 1.f)) + second * ratio};
    }
}