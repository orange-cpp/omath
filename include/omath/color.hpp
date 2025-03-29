//
// Created by vlad on 2/4/24.
//

#pragma once

#include <cstdint>
#include "omath/vector3.hpp"
#include "omath/vector4.hpp"

#ifdef max
#undef max
#endif


#ifdef min
#undef min
#endif

namespace omath
{
    struct HSV
    {
        float hue{};
        float saturation{};
        float value{};
    };


    class Color final : public Vector4<float>
    {
    public:
        constexpr Color(const float r, const float g, const float b, const float a) : Vector4(r, g, b, a)
        {
            Clamp(0.f, 1.f);
        }

        constexpr explicit Color() = default;
        [[nodiscard]]
        constexpr static Color FromRGBA(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
        {
            return Color{Vector4(r, g, b, a) / 255.f};
        }

        [[nodiscard]]
        constexpr static Color FromHSV(float hue, const float saturation, const float value)
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
                case 0:
                    r = value, g = t, b = p;
                    break;
                case 1:
                    r = q, g = value, b = p;
                    break;
                case 2:
                    r = p, g = value, b = t;
                    break;
                case 3:
                    r = p, g = q, b = value;
                    break;
                case 4:
                    r = t, g = p, b = value;
                    break;
                case 5:
                    r = value, g = p, b = q;
                    break;

                default:
                    return {0.f, 0.f, 0.f, 0.f};
            }

            return {r, g, b, 1.f};
        }

        [[nodiscard]]
        constexpr static Color FromHSV(const HSV& hsv)
        {
            return FromHSV(hsv.hue, hsv.saturation, hsv.value);
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
                hsvData.hue = 0.f;

            else if (max == red)
                hsvData.hue = 60.f * (std::fmodf(((green - blue) / delta), 6.f));
            else if (max == green)
                hsvData.hue = 60.f * (((blue - red) / delta) + 2.f);
            else if (max == blue)
                hsvData.hue = 60.f * (((red - green) / delta) + 4.f);

            if (hsvData.hue < 0.f)
                hsvData.hue += 360.f;

            hsvData.hue /= 360.f;
            hsvData.saturation = max == 0.f ? 0.f : delta / max;
            hsvData.value = max;

            return hsvData;
        }

        constexpr explicit Color(const Vector4& vec) : Vector4(vec)
        {
            Clamp(0.f, 1.f);
        }
        constexpr void SetHue(const float hue)
        {
            auto hsv = ToHSV();
            hsv.hue = hue;

            *this = FromHSV(hsv);
        }

        constexpr void SetSaturation(const float saturation)
        {
            auto hsv = ToHSV();
            hsv.saturation = saturation;

            *this = FromHSV(hsv);
        }

        constexpr void SetValue(const float value)
        {
            auto hsv = ToHSV();
            hsv.value = value;

            *this = FromHSV(hsv);
        }
        [[nodiscard]]
        constexpr Color Blend(const Color& other, float ratio) const
        {
            ratio = std::clamp(ratio, 0.f, 1.f);
            return Color(*this * (1.f - ratio) + other * ratio);
        }

        [[nodiscard]] static constexpr Color Red()
        {
            return {1.f, 0.f, 0.f, 1.f};
        }
        [[nodiscard]] static constexpr Color Green()
        {
            return {0.f, 1.f, 0.f, 1.f};
        }
        [[nodiscard]] static constexpr Color Blue()
        {
            return {0.f, 0.f, 1.f, 1.f};
        }
    };
} // namespace omath
