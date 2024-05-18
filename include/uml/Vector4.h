//
// Vector4.h
//
#pragma once

#include <uml/Vector3.h>

namespace uml
{
    class Vector4 : public Vector3
    {
    public:
        float w = 0.f;

        Vector4(const float x = 0.f, const float y = 0.f, const float z = 0.f, const float w = 0.f) : Vector3(x, y, z), w(w) {}
        Vector4() = default;

        [[nodiscard]]
        bool operator==(const Vector4& src) const;

        [[nodiscard]]
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
        Vector4& Clamp(float min, float max);

        [[nodiscard]]
        Vector4 operator-() const;

        [[nodiscard]]
        Vector4 operator+(const Vector4& v) const;

        [[nodiscard]]
        Vector4 operator-(const Vector4& v) const;

        [[nodiscard]]
        Vector4 operator*(float scalar) const;

        [[nodiscard]]
        Vector4 operator*(const Vector4& v) const;

        [[nodiscard]]
        Vector4 operator/(float scalar) const;

        [[nodiscard]]
        Vector4 operator/(const Vector4& v) const;

        [[nodiscard]]
        float Sum() const;
    };
}
