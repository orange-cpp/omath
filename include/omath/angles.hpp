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
    [[nodiscard]] constexpr Type radians_to_degrees(const Type& radians)
    {
        return radians * (Type(180) / std::numbers::pi_v<Type>);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]] constexpr Type degrees_to_radians(const Type& degrees)
    {
        return degrees * (std::numbers::pi_v<Type> / Type(180));
    }

    template<class type>
    requires std::is_floating_point_v<type>
    [[nodiscard]] type horizontal_fov_to_vertical(const type& horizontal_fov, const type& aspect)
    {
        const auto fov_rad = degrees_to_radians(horizontal_fov);

        const auto vert_fov = type(2) * std::atan(std::tan(fov_rad / type(2)) / aspect);

        return radians_to_degrees(vert_fov);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]] Type vertical_fov_to_horizontal(const Type& vertical_fov, const Type& aspect)
    {
        const auto fov_as_radians = degrees_to_radians(vertical_fov);

        const auto horizontal_fov = Type(2) * std::atan(std::tan(fov_as_radians / Type(2)) * aspect);

        return radians_to_degrees(horizontal_fov);
    }

    template<class Type>
    requires std::is_arithmetic_v<Type>
    [[nodiscard]] Type wrap_angle(const Type& angle, const Type& min, const Type& max)
    {
        if (angle <= max && angle >= min)
            return angle;

        const Type range = max - min;

        Type wrappedAngle = std::fmod(angle - min, range);

        if (wrappedAngle < 0)
            wrappedAngle += range;

        return wrappedAngle + min;
    }
} // namespace omath::angles
