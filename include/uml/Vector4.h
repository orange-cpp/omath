//
// Vector4.h
//
#pragma once

#include <uml/Vector3.h>

namespace uml
{
    class Vector4 final : public Vector3
    {
    public:
        float w = 0.f;

        Vector4(float x = 0.f, float y = 0.f, float z = 0.f, float w = 0.f) : Vector3(x, y, z), w(w) {}
        Vector4() = default;

        bool operator==(const Vector4& src) const;
        bool operator!=(const Vector4& src) const;

        Vector4& operator+=(const Vector4& v);
        Vector4& operator-=(const Vector4& v);
        Vector4& operator*=(float scalar);
        Vector4& operator*=(const Vector4& v);
        Vector4& operator/=(float scalar);
        Vector4& operator/=(const Vector4& v);

        [[nodiscard]] float Length() const;
        [[nodiscard]] float LengthSqr() const;
        [[nodiscard]] float Dot(const Vector4& vOther) const;
        Vector4& Abs();
        Vector4 operator-() const;
        Vector4 operator+(const Vector4& v) const;
        Vector4 operator-(const Vector4& v) const;
        Vector4 operator*(float scalar) const;
        Vector4 operator*(const Vector4& v) const;
        Vector4 operator/(float scalar) const;
        Vector4 operator/(const Vector4& v) const;

        [[nodiscard]] float Sum() const;
    };
}
