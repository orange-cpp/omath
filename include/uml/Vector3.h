//
// Created by vlad on 10/28/23.
//

#pragma once

namespace uml
{
    class Vector3 final
    {
    public:
        float x = 0.f;
        float y = 0.f;
        float z = 0.f;
        Vector3(const float x, const float y, const float z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
        }
        Vector3() = default;

        bool operator==(const Vector3& src) const;
        bool operator!=(const Vector3& src) const;

        Vector3& operator+=(const Vector3& v);
        Vector3& operator-=(const Vector3& v);
        Vector3& operator*=(float fl);
        Vector3& operator*=(const Vector3& v);
        Vector3& operator/=(const Vector3& v);
        Vector3& operator+=(float fl);
        Vector3& operator/=(float fl);
        Vector3& operator-=(float fl);

        [[nodiscard]] float DistTo(const Vector3& vOther) const;
        Vector3& Abs();
        [[nodiscard]] float DistToSqr(const Vector3& vOther) const;
        [[nodiscard]] float Dot(const Vector3& vOther) const;
        [[nodiscard]] float Length() const;
        [[nodiscard]] float LengthSqr() const;
        [[nodiscard]] float Length2D() const;

        Vector3 operator-() const;
        Vector3 operator+(const Vector3& v) const;
        Vector3 operator-(const Vector3& v) const;
        Vector3 operator*(float fl) const;
        Vector3 operator*(const Vector3& v) const;
        Vector3 operator/(float fl) const;
        Vector3 operator/(const Vector3& v) const;
        [[nodiscard]] static Vector3 CreateVelocity(const Vector3& angles, const float length) ;
        [[nodiscard]] float  Sum() const;
        [[nodiscard]] float  Sum2D() const;
        [[nodiscard]] Vector3 ViewAngleTo(const Vector3& other) const;
    };
}
