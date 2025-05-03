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
        return radians * (static_cast<Type>(180) / std::numbers::pi_v<Type>);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]] constexpr Type degrees_to_radians(const Type& degrees)
    {
        return degrees * (std::numbers::pi_v<Type> / static_cast<Type>(180));
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]] Type horizontal_fov_to_vertical(const Type& horizontal_fov, const Type& aspect)
    {
        const auto fov_rad = degrees_to_radians(horizontal_fov);

        const auto vert_fov = static_cast<Type>(2) * std::atan(std::tan(fov_rad / static_cast<Type>(2)) / aspect);

        return radians_to_degrees(vert_fov);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]] Type vertical_fov_to_horizontal(const Type& vertical_fov, const Type& aspect)
    {
        const auto fov_as_radians = degrees_to_radians(vertical_fov);

        const auto horizontal_fov
                = static_cast<Type>(2) * std::atan(std::tan(fov_as_radians / static_cast<Type>(2)) * aspect);

        return radians_to_degrees(horizontal_fov);
    }

    template<class Type>
    requires std::is_arithmetic_v<Type>
    [[nodiscard]] Type wrap_angle(const Type& angle, const Type& min, const Type& max)
    {
        if (angle <= max && angle >= min)
            return angle;

        const Type range = max - min;

        Type wrapped_angle = std::fmod(angle - min, range);

        if (wrapped_angle < 0)
            wrapped_angle += range;

        return wrapped_angle + min;
    }
} // namespace omath::angles
