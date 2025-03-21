//
// Created by vlad on 11/6/23.
//

#pragma once
#include <cmath>
#include <numbers>

namespace omath::angles
{
    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]] constexpr Type RadiansToDegrees(const Type& radians)
    {
        return radians * (Type(180) / std::numbers::pi_v<Type>);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]] constexpr Type DegreesToRadians(const Type& degrees)
    {
        return degrees * (std::numbers::pi_v<Type> / Type(180));
    }

    template<class type>
    requires std::is_floating_point_v<type>
    [[nodiscard]] type HorizontalFovToVertical(const type& horFov, const type& aspect)
    {
        const auto fovRad = DegreesToRadians(horFov);

        const auto vertFov = type(2) * std::atan(std::tan(fovRad / type(2)) / aspect);

        return RadiansToDegrees(vertFov);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]] Type VerticalFovToHorizontal(const Type& vertFov, const Type& aspect)
    {
        const auto fovRad = DegreesToRadians(vertFov);

        const auto horFov = Type(2) * std::atan(std::tan(fovRad / Type(2)) * aspect);

        return RadiansToDegrees(horFov);
    }

    template<class Type>
    requires std::is_arithmetic_v<Type>
    [[nodiscard]] Type WrapAngle(const Type& angle, const Type& min, const Type& max)
    {
        if (angle <= max && angle >= min)
            return angle;

        const Type range = max - min;

        Type wrappedAngle = std::fmod(angle - min, range);

        if (wrappedAngle < 0)
            wrappedAngle += range;

        return wrappedAngle + min;
    }
}
