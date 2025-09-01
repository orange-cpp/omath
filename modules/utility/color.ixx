//
// Created by Vlad on 9/1/2025.
//
module;
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <format>

#ifdef OMATH_IMGUI_INTEGRATION
#include <imgui.h>
#endif

export module omath.color;
import omath.vector4;

export namespace omath
{
    struct Hsv
    {
        float hue{};
        float saturation{};
        float value{};
    };

    class Color final : public Vector4<float>
    {
    public:
        constexpr Color(const float r, const float g, const float b, const float a) noexcept: Vector4(r, g, b, a)
        {
            clamp(0.f, 1.f);
        }

        constexpr explicit Color() noexcept = default;
        [[nodiscard]]
        constexpr static Color from_rgba(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) noexcept
        {
            return Color{Vector4(r, g, b, a) / 255.f};
        }

        [[nodiscard]]
        constexpr static Color from_hsv(float hue, const float saturation, const float value) noexcept
        {
            float r{}, g{}, b{};

            hue = std::clamp(hue, 0.f, 1.f);

            const int i = static_cast<int>(hue * 6.f);
            const float f = hue * 6.f - static_cast<float>(i);
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
        constexpr static Color from_hsv(const Hsv& hsv) noexcept
        {
            return from_hsv(hsv.hue, hsv.saturation, hsv.value);
        }

        [[nodiscard]]
        constexpr Hsv to_hsv() const noexcept
        {
            Hsv hsv_data;

            const float& red = x;
            const float& green = y;
            const float& blue = z;

            const float max = std::max({red, green, blue});
            const float min = std::min({red, green, blue});
            const float delta = max - min;

            if (delta == 0.f)
                hsv_data.hue = 0.f;

            else if (max == red)
                hsv_data.hue = 60.f * (std::fmodf(((green - blue) / delta), 6.f));
            else if (max == green)
                hsv_data.hue = 60.f * (((blue - red) / delta) + 2.f);
            else if (max == blue)
                hsv_data.hue = 60.f * (((red - green) / delta) + 4.f);

            if (hsv_data.hue < 0.f)
                hsv_data.hue += 360.f;

            hsv_data.hue /= 360.f;
            hsv_data.saturation = max == 0.f ? 0.f : delta / max;
            hsv_data.value = max;

            return hsv_data;
        }

        constexpr explicit Color(const Vector4& vec) noexcept: Vector4(vec)
        {
            clamp(0.f, 1.f);
        }
        constexpr void set_hue(const float hue) noexcept
        {
            auto hsv = to_hsv();
            hsv.hue = hue;

            *this = from_hsv(hsv);
        }

        constexpr void set_saturation(const float saturation) noexcept
        {
            auto hsv = to_hsv();
            hsv.saturation = saturation;

            *this = from_hsv(hsv);
        }

        constexpr void set_value(const float value) noexcept
        {
            auto hsv = to_hsv();
            hsv.value = value;

            *this = from_hsv(hsv);
        }
        [[nodiscard]]
        constexpr Color blend(const Color& other, float ratio) const noexcept
        {
            ratio = std::clamp(ratio, 0.f, 1.f);
            return Color(*this * (1.f - ratio) + other * ratio);
        }

        [[nodiscard]] static constexpr Color red()
        {
            return {1.f, 0.f, 0.f, 1.f};
        }
        [[nodiscard]] static constexpr Color green()
        {
            return {0.f, 1.f, 0.f, 1.f};
        }
        [[nodiscard]] static constexpr Color blue()
        {
            return {0.f, 0.f, 1.f, 1.f};
        }
#ifdef OMATH_IMGUI_INTEGRATION
        [[nodiscard]]
        ImColor to_im_color() const noexcept
        {
            return {to_im_vec4()};
        }
#endif
    };
} // namespace omath
export template<>
struct std::formatter<omath::Color> // NOLINT(*-dcl58-cpp)
{
    [[nodiscard]]
    static constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }
    [[nodiscard]]
    static auto format(const omath::Color& col, std::format_context& ctx)
    {
        return std::format_to(ctx.out(), "[r:{}, g:{}, b:{}, a:{}]",
            static_cast<int>(col.x * 255.f),
            static_cast<int>(col.y * 255.f),
            static_cast<int>(col.z * 255.f),
            static_cast<int>(col.w * 255.f));
    }
};