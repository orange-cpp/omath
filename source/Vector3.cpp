//
// Created by vlad on 10/28/23.
//


#include <uml/Vector3.h>
#include <cmath>
#include <uml/angles.h>


namespace uml
{
    bool Vector3::operator==(const Vector3 &src) const
    {
        return (src.x == x) and (src.y == y) and (src.z == z);
    }

    bool Vector3::operator!=(const Vector3 &src) const
    {
        return (src.x != x) or (src.y != y) or (src.z != z);
    }

    Vector3 &Vector3::operator+=(const Vector3 &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;

        return *this;
    }

    Vector3 &Vector3::operator-=(const Vector3 &v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;

        return *this;
    }

    Vector3 &Vector3::operator*=(const float fl)
    {
        x *= fl;
        y *= fl;
        z *= fl;

        return *this;
    }

    Vector3 &Vector3::operator*=(const Vector3 &v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;

        return *this;
    }

    Vector3 &Vector3::operator/=(const Vector3 &v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;

        return *this;
    }

    Vector3 &Vector3::operator+=(const float fl)
    {
        x += fl;
        y += fl;
        z += fl;

        return *this;
    }

    Vector3 &Vector3::operator/=(const float fl)
    {
        x /= fl;
        y /= fl;
        z /= fl;

        return *this;
    }

    Vector3 &Vector3::operator-=(const float fl)
    {
        x -= fl;
        y -= fl;
        z -= fl;

        return *this;
    }

    float Vector3::DistTo(const Vector3 &vOther) const
    {
        Vector3 delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;
        delta.z = z - vOther.z;

        return delta.Length();
    }

    Vector3 &Vector3::Abs()
    {
        x = fabsf(x);
        y = fabsf(y);
        z = fabsf(z);

        return *this;
    }

    float Vector3::DistToSqr(const Vector3 &vOther) const
    {
        Vector3 delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;
        delta.z = z - vOther.z;

        return delta.LengthSqr();
    }

    float Vector3::Dot(const Vector3 &vOther) const
    {
        return (x * vOther.x + y * vOther.y + z * vOther.z);
    }

    float Vector3::Length() const
    {
        return sqrt(x * x + y * y + z * z);
    }

    float Vector3::LengthSqr(void) const
    {
        return (x * x + y * y + z * z);
    }

    float Vector3::Length2D() const
    {
        return sqrt(x * x + y * y);

    }

    Vector3 Vector3::operator-(void) const
    {
        return {-x, -y, -z};
    }

    Vector3 Vector3::operator+(const Vector3 &v) const
    {
        return {x + v.x, y + v.y, z + v.z};
    }

    Vector3 Vector3::operator-(const Vector3 &v) const
    {
        return {x - v.x, y - v.y, z - v.z};
    }

    Vector3 Vector3::operator*(float fl) const
    {
        return {x * fl, y * fl, z * fl};
    }

    Vector3 Vector3::operator*(const Vector3 &v) const
    {
        return {x * v.x, y * v.y, z * v.z};
    }

    Vector3 Vector3::operator/(const float fl) const
    {
        return {x / fl, y / fl, z / fl};
    }

    Vector3 Vector3::operator/(const Vector3 &v) const
    {
        return {x / v.x, y / v.y, z / v.z};
    }

    Vector3 Vector3::Transform(const Vector3 &angles, const float length) const
    {
        Vector3 transformed;
        transformed.x += cosf(angles.y * static_cast<float>(M_PI / 180.f)) * length;
        transformed.y += sinf(angles.y * static_cast<float>(M_PI / 180.f)) * length;
        transformed.z += tanf(angles.x * static_cast<float>(M_PI / 180.f)) * length;

        return transformed;
    }

    float Vector3::Sum() const
    {
        return x + y + z;
    }

    float Vector3::Sum2D() const
    {
        return x + y;
    }

    Vector3 Vector3::ViewAngleTo(const Vector3 &other) const
    {
        const float distance = DistTo(other);
        const auto delta = other - *this;

        // Make x negative since -89 is top and 89 is bottom
        return
        {
            -angles::RadToDeg(asinf(delta.z / distance)),
            angles::RadToDeg(atan2f(delta.y, delta.x)),
            0.f
        };
    }
}