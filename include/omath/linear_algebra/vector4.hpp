//
// Vector4.h
//
#pragma once

#include "omath/linear_algebra/vector3.hpp"
#include <algorithm>

namespace omath
{
    template<class Type>
    requires std::is_arithmetic_v<Type>
    class Vector4 : public Vector3<Type>
    {
    public:
        using ContainedType = Type;
        Type w = static_cast<Type>(0);

        constexpr Vector4(const Type& x, const Type& y, const Type& z, const Type& w): Vector3<Type>(x, y, z), w(w)
        {
        }
        constexpr Vector4() noexcept: Vector3<Type>(), w(static_cast<Type>(0)) {};


        template<class CastedType>
        requires std::is_arithmetic_v<CastedType>
        [[nodiscard("You must use casted vector")]] constexpr explicit operator Vector4<CastedType>() const noexcept
        {
            return {static_cast<CastedType>(this->x), static_cast<CastedType>(this->y),
                    static_cast<CastedType>(this->z), static_cast<CastedType>(this->w)};
        }

        [[nodiscard("You must use comparison result")]]
        constexpr bool operator==(const Vector4& other) const noexcept
        {
            return Vector3<Type>::operator==(other) && w == other.w;
        }

        [[nodiscard("You must use comparison result")]]
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

        constexpr Vector4& operator+=(const Type& value) noexcept
        {
            Vector3<Type>::operator+=(value);
            w += value;

            return *this;
        }

        constexpr Vector4& operator-=(const Type& value) noexcept
        {
            Vector3<Type>::operator-=(value);
            w -= value;

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
            w /= value;

            return *this;
        }

        constexpr Vector4& operator/=(const Vector4& other) noexcept
        {
            Vector3<Type>::operator/=(other);
            w /= other.w;
            return *this;
        }

        [[nodiscard("You must use squared length")]]
        constexpr Type length_sqr() const noexcept
        {
            return Vector3<Type>::length_sqr() + w * w;
        }

        [[nodiscard("You must use dot product")]]
        constexpr Type dot(const Vector4& other) const noexcept
        {
            return Vector3<Type>::dot(other) + w * other.w;
        }

        [[nodiscard("You must use length")]]
        constexpr Type length() const noexcept
        requires std::is_floating_point_v<Type>
        {
            return internal::hypot(this->x, this->y, this->z, w);
        }

        [[nodiscard("You must use squared distance")]]
        constexpr Type distance_to_sqr(const Vector4& other) const noexcept
        {
            return (*this - other).length_sqr();
        }

        [[nodiscard("You must use distance")]]
        constexpr Type distance_to(const Vector4& other) const noexcept
        requires std::is_floating_point_v<Type>
        {
            return (*this - other).length();
        }

        [[nodiscard("You must use normalized vector")]]
        constexpr Vector4 normalized() const noexcept
        requires std::is_floating_point_v<Type>
        {
            const Type len = length();
            return len != static_cast<Type>(0) ? *this / len : *this;
        }

        constexpr Vector4& abs() noexcept
        {
            Vector3<Type>::abs();
            w = w < static_cast<Type>(0) ? -w : w;

            return *this;
        }
        [[nodiscard("You must use absed vector")]]
        constexpr Vector4 abs() const noexcept
        {
            return Vector4{*this}.abs();
        }
        constexpr Vector4& clamp(const Type& min, const Type& max) noexcept
        {
            this->x = std::clamp(this->x, min, max);
            this->y = std::clamp(this->y, min, max);
            this->z = std::clamp(this->z, min, max);
            w = std::clamp(w, min, max);

            return *this;
        }

        [[nodiscard("You must use negated vector")]]
        constexpr Vector4 operator-() const noexcept
        {
            return {-this->x, -this->y, -this->z, -w};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector4 operator+(const Vector4& other) const noexcept
        {
            return {this->x + other.x, this->y + other.y, this->z + other.z, w + other.w};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector4 operator-(const Vector4& other) const noexcept
        {
            return {this->x - other.x, this->y - other.y, this->z - other.z, w - other.w};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector4 operator*(const Type& value) const noexcept
        {
            return {this->x * value, this->y * value, this->z * value, w * value};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector4 operator*(const Vector4& other) const noexcept
        {
            return {this->x * other.x, this->y * other.y, this->z * other.z, w * other.w};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector4 operator/(const Type& value) const noexcept
        {
            return {this->x / value, this->y / value, this->z / value, w / value};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector4 operator/(const Vector4& other) const noexcept
        {
            return {this->x / other.x, this->y / other.y, this->z / other.z, w / other.w};
        }

        [[nodiscard("You must use sum of elements")]]
        constexpr Type sum() const noexcept
        {
            return Vector3<Type>::sum() + w;
        }

        [[nodiscard("You must use direction check result")]]
        constexpr bool point_to_same_direction(const Vector4& other) const noexcept
        {
            return dot(other) > static_cast<Type>(0);
        }

        // NOTE: SelfType defaults to Type so the return type is only formed when the function is called. Naming
        // Angle directly would instantiate it for integer vectors and violate its floating point constraint.
        template<class SelfType = Type>
        requires std::is_same_v<SelfType, Type> && std::is_floating_point_v<SelfType>
        [[nodiscard("You must use angle between vectors")]]
        constexpr std::expected<Angle<SelfType, SelfType{0}, SelfType{180}, AngleFlags::Clamped>, Vector3Error>
        angle_between(const Vector4& other) const noexcept
        {
            const auto bottom = length() * other.length();

            if (bottom == static_cast<Type>(0))
                return std::unexpected(Vector3Error::IMPOSSIBLE_BETWEEN_ANGLE);
            return Angle<SelfType, SelfType{0}, SelfType{180}, AngleFlags::Clamped>::from_radians(
                    internal::acos(dot(other) / bottom));
        }

        [[nodiscard("You must use perpendicularity check result")]]
        constexpr bool is_perpendicular(const Vector4& other, Type epsilon = static_cast<Type>(0.0001)) const noexcept
        requires std::is_floating_point_v<Type>
        {
            if (const auto angle = angle_between(other))
                return internal::abs(angle->as_degrees() - static_cast<Type>(90)) <= epsilon;

            return false;
        }

        // NOTE: Ordering by squared length, sqrt is monotonic so the order is identical without the cost.
        [[nodiscard("You must use comparison result")]]
        constexpr bool operator<(const Vector4& other) const noexcept
        {
            return length_sqr() < other.length_sqr();
        }

        [[nodiscard("You must use comparison result")]]
        constexpr bool operator>(const Vector4& other) const noexcept
        {
            return length_sqr() > other.length_sqr();
        }

        [[nodiscard("You must use comparison result")]]
        constexpr bool operator<=(const Vector4& other) const noexcept
        {
            return length_sqr() <= other.length_sqr();
        }

        [[nodiscard("You must use comparison result")]]
        constexpr bool operator>=(const Vector4& other) const noexcept
        {
            return length_sqr() >= other.length_sqr();
        }

        [[nodiscard("You must use array")]]
        constexpr std::array<Type, 4> as_array() const noexcept
        {
            return {this->x, this->y, this->z, w};
        }

#ifdef OMATH_IMGUI_INTEGRATION
        [[nodiscard("You must use ImVec4")]]
        constexpr ImVec4 to_im_vec4() const noexcept
        {
            return {
                    static_cast<float>(this->x),
                    static_cast<float>(this->y),
                    static_cast<float>(this->z),
                    static_cast<float>(w),
            };
        }
        [[nodiscard("You must use vector from ImVec4")]]
        static Vector4 from_im_vec4(const ImVec4& other) noexcept
        {
            return {static_cast<Type>(other.x), static_cast<Type>(other.y), static_cast<Type>(other.z),
                    static_cast<Type>(other.w)};
        }
#endif
    };
} // namespace omath

template<class Type>
struct std::hash<omath::Vector4<Type>> final // NOLINT(*-dcl58-cpp)
{
    [[nodiscard("You must use hash value")]]
    std::size_t operator()(const omath::Vector4<Type>& vec) const noexcept
    {
        std::size_t hash = 0;
        constexpr std::hash<Type> hasher;

        hash ^= hasher(vec.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= hasher(vec.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= hasher(vec.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= hasher(vec.w) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        return hash;
    }
};
template<class Type>
struct std::formatter<omath::Vector4<Type>> final // NOLINT(*-dcl58-cpp)
{
    [[nodiscard("You must use parse iterator")]]
    static constexpr auto parse(std::format_parse_context& ctx) noexcept
    {
        return ctx.begin();
    }
    template<class FormatContext>
    [[nodiscard("You must use format iterator")]]
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
