//
// Created by vlad on 10/28/23.
//

#include <omath/Vector3.hpp>
#include <cmath>
#include <omath/Angles.hpp>

namespace omath
{
    Vector3 Vector3::ViewAngleTo(const Vector3 &other) const
    {
        const float distance = DistTo(other);
        const auto delta = other - *this;

        return
        {
            angles::RadiansToDegrees(std::asin(delta.z / distance)),
            angles::RadiansToDegrees(std::atan2(delta.y, delta.x)),
            0.f
        };
    }
}