//
// Vector4.h
//
#pragma once

#include <algorithm>
#include "omath/linear_algebra/vector3.hpp"

namespace omath
{
    template<class Type>
    requires std::is_arithmetic_v<Type>
    class Vector4 : public Vector3<Type>
    {
    public:
        using ContainedType = Type;
        Type w;

        constexpr Vector4(const Type& x, const Type& y, const Type& z, const Type& w): Vector3<Type>(x, y, z), w(w)
        {
        }
        constexpr Vector4() noexcept: Vector3<Type>(), w(static_cast<Type>(0)) {};

        [[nodiscard]]
        constexpr bool operator==(const Vector4& other) const noexcept
        {
            return Vector3<Type>::operator==(other) && w == other.w;
        }

        [[nodiscard]]
        constexpr bool operator!=(const Vector4& other) const noexcept
        {
            return !(*this == other);
        }

        constexpr Vector4& operator+=(const Vector4& other) noexcept
        {
            Vector3<Type>::operator+=(other);
            w += other.w;

            return *this;
        }

        constexpr Vector4& operator-=(const Vector4& other) noexcept
        {
            Vector3<Type>::operator-=(other);
            w -= other.w;

            return *this;
        }

        constexpr Vector4& operator*=(const Type& value) noexcept
        {
            Vector3<Type>::operator*=(value);
            w *= value;

            return *this;
        }

        constexpr Vector4& operator*=(const Vector4& other) noexcept
        {
            Vector3<Type>::operator*=(other);
            w *= other.w;

            return *this;
        }

        constexpr Vector4& operator/=(const Type& value) noexcept
        {
            Vector3<Type>::operator/=(value);
            if constexpr (std::is_floating_point_v<Type>)
            {
                if (value == static_cast<Type>(0))
                {
                    const Type inf = std::numeric_limits<Type>::infinity();
                    const Type nan = std::numeric_limits<Type>::quiet_NaN();
                    w = (w == static_cast<Type>(0)) ? nan : ((w > static_cast<Type>(0)) ? inf : -inf);
                }
                else
                {
                    w /= value;
                }
            }
            else
            {
                w /= value;
            }

            return *this;
        }

        constexpr Vector4& operator/=(const Vector4& other) noexcept
        {
            Vector3<Type>::operator/=(other);
            if constexpr (std::is_floating_point_v<Type>)
            {
                if (other.w == static_cast<Type>(0))
                {
                    const Type inf = std::numeric_limits<Type>::infinity();
                    const Type nan = std::numeric_limits<Type>::quiet_NaN();
                    w = (w == static_cast<Type>(0)) ? nan : ((w > static_cast<Type>(0)) ? inf : -inf);
                }
                else
                {
                    w /= other.w;
                }
            }
            else
            {
                w /= other.w;
            }
            return *this;
        }

        [[nodiscard]] constexpr Type length_sqr() const noexcept
        {
            return Vector3<Type>::length_sqr() + w * w;
        }

        [[nodiscard]] constexpr Type dot(const Vector4& other) const noexcept
        {
            return Vector3<Type>::dot(other) + w * other.w;
        }

        [[nodiscard]] Type length() const noexcept
        {
            return std::sqrt(length_sqr());
        }

        constexpr Vector4& abs() noexcept
        {
            Vector3<Type>::abs();
            w = w < 0.f ? -w : w;

            return *this;
        }
        constexpr Vector4& clamp(const Type& min, const Type& max) noexcept
        {
            this->x = std::clamp(this->x, min, max);
            this->y = std::clamp(this->y, min, max);
            this->z = std::clamp(this->z, min, max);

            return *this;
        }

        [[nodiscard]]
        constexpr Vector4 operator-() const noexcept
        {
            return {-this->x, -this->y, -this->z, -w};
        }

        [[nodiscard]]
        constexpr Vector4 operator+(const Vector4& other) const noexcept
        {
            return {this->x + other.x, this->y + other.y, this->z + other.z, w + other.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator-(const Vector4& other) const noexcept
        {
            return {this->x - other.x, this->y - other.y, this->z - other.z, w - other.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator*(const Type& value) const noexcept
        {
            return {this->x * value, this->y * value, this->z * value, w * value};
        }

        [[nodiscard]]
        constexpr Vector4 operator*(const Vector4& other) const noexcept
        {
            return {this->x * other.x, this->y * other.y, this->z * other.z, w * other.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator/(const Type& value) const noexcept
        {
            if constexpr (std::is_floating_point_v<Type>)
            {
                if (value == static_cast<Type>(0))
                {
                    const Type inf = std::numeric_limits<Type>::infinity();
                    const Type nan = std::numeric_limits<Type>::quiet_NaN();
                    return {
                        (this->x == static_cast<Type>(0)) ? nan : ((this->x > static_cast<Type>(0)) ? inf : -inf),
                        (this->y == static_cast<Type>(0)) ? nan : ((this->y > static_cast<Type>(0)) ? inf : -inf),
                        (this->z == static_cast<Type>(0)) ? nan : ((this->z > static_cast<Type>(0)) ? inf : -inf),
                        (w == static_cast<Type>(0)) ? nan : ((w > static_cast<Type>(0)) ? inf : -inf)
                    };
                }
            }
            return {this->x / value, this->y / value, this->z / value, w / value};
        }

        [[nodiscard]]
        constexpr Vector4 operator/(const Vector4& other) const noexcept
        {
            if constexpr (std::is_floating_point_v<Type>)
            {
                const Type inf = std::numeric_limits<Type>::infinity();
                const Type nan = std::numeric_limits<Type>::quiet_NaN();
                return {
                    (other.x == static_cast<Type>(0)) ? ((this->x == static_cast<Type>(0)) ? nan : ((this->x > static_cast<Type>(0)) ? inf : -inf)) : this->x / other.x,
                    (other.y == static_cast<Type>(0)) ? ((this->y == static_cast<Type>(0)) ? nan : ((this->y > static_cast<Type>(0)) ? inf : -inf)) : this->y / other.y,
                    (other.z == static_cast<Type>(0)) ? ((this->z == static_cast<Type>(0)) ? nan : ((this->z > static_cast<Type>(0)) ? inf : -inf)) : this->z / other.z,
                    (other.w == static_cast<Type>(0)) ? ((w == static_cast<Type>(0)) ? nan : ((w > static_cast<Type>(0)) ? inf : -inf)) : w / other.w
                };
            }
            return {this->x / other.x, this->y / other.y, this->z / other.z, w / other.w};
        }

        [[nodiscard]]
        constexpr Type sum() const noexcept
        {
            return Vector3<Type>::sum() + w;
        }

        [[nodiscard]]
        bool operator<(const Vector4& other) const noexcept
        {
            return length() < other.length();
        }

        [[nodiscard]]
        bool operator>(const Vector4& other) const noexcept
        {
            return length() > other.length();
        }

        [[nodiscard]]
        bool operator<=(const Vector4& other) const noexcept
        {
            return length() <= other.length();
        }

        [[nodiscard]]
        bool operator>=(const Vector4& other) const noexcept
        {
            return length() >= other.length();
        }

#ifdef OMATH_IMGUI_INTEGRATION
        [[nodiscard]]
        constexpr ImVec4 to_im_vec4() const noexcept
        {
            return {
                    static_cast<float>(this->x),
                    static_cast<float>(this->y),
                    static_cast<float>(this->z),
                    static_cast<float>(w),
            };
        }
        [[nodiscard]]
        static Vector4<float> from_im_vec4(const ImVec4& other) noexcept
        {
            return {static_cast<Type>(other.x), static_cast<Type>(other.y), static_cast<Type>(other.z)};
        }
#endif
};
} // namespace omath

template<> struct std::hash<omath::Vector4<float>>
{
    [[nodiscard]]
    std::size_t operator()(const omath::Vector4<float>& vec) const noexcept
    {
        std::size_t hash = 0;
        constexpr std::hash<float> hasher;

        hash ^= hasher(vec.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= hasher(vec.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= hasher(vec.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= hasher(vec.w) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        return hash;
    }
};

template<class Type>
struct std::formatter<omath::Vector4<Type>> // NOLINT(*-dcl58-cpp)
{
    [[nodiscard]]
    static constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }
    template<class FormatContext>
    [[nodiscard]]
    static auto format(const omath::Vector4<Type>& vec, FormatContext& ctx)
    {
        if constexpr (std::is_same_v<typename FormatContext::char_type, char>)
            return std::format_to(ctx.out(), "[{}, {}, {}, {}]", vec.x, vec.y, vec.z, vec.w);

        if constexpr (std::is_same_v<typename FormatContext::char_type, wchar_t>)
            return std::format_to(ctx.out(), L"[{}, {}, {}, {}]", vec.x, vec.y, vec.z, vec.w);

        if constexpr (std::is_same_v<typename FormatContext::char_type, char8_t>)
            return std::format_to(ctx.out(), u8"[{}, {}, {}, {}]", vec.x, vec.y, vec.z, vec.w);
    }
};