//
// Created by Vlad on 02.09.2024.
//

#pragma once
#include <tuple>


namespace omath
{
    class Vector2
    {
    public:
        float x = 0.f;
        float y = 0.f;

        // Constructors
        Vector2() = default;
        Vector2(float x, float y);

        // Equality operators
        bool operator==(const Vector2& src) const;
        bool operator!=(const Vector2& src) const;

        // Compound assignment operators
        Vector2& operator+=(const Vector2& v);
        Vector2& operator-=(const Vector2& v);
        Vector2& operator*=(const Vector2& v);
        Vector2& operator/=(const Vector2& v);

        Vector2& operator*=(float fl);
        Vector2& operator/=(float fl);
        Vector2& operator+=(float fl);
        Vector2& operator-=(float fl);

        // Basic vector operations
        [[nodiscard]] float DistTo(const Vector2& vOther) const;
        [[nodiscard]] float DistToSqr(const Vector2& vOther) const;
        [[nodiscard]] float Dot(const Vector2& vOther) const;
        [[nodiscard]] float Length() const;
        [[nodiscard]] float LengthSqr() const;
        Vector2& Abs();

        template<class type>
        const type& As() const
        {
            return *reinterpret_cast<const type*>(this);
        }
        template<class type>
        type& As()
        {
            return *reinterpret_cast<type*>(this);
        }

        // Unary negation operator
        Vector2 operator-() const;

        // Binary arithmetic operators
        Vector2 operator+(const Vector2& v) const;
        Vector2 operator-(const Vector2& v) const;
        Vector2 operator*(float fl) const;
        Vector2 operator/(float fl) const;


        // Normalize the vector
        [[nodiscard]] Vector2 Normalized() const;

        // Sum of elements
        [[nodiscard]] float Sum() const;

        [[nodiscard]]
        std::tuple<float, float> AsTuple() const;
    };
}