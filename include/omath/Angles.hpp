//
// Created by vlad on 11/6/23.
//

#pragma once
#include <numbers>

namespace omath::angles
{
    template<class type>
    requires std::is_floating_point_v<type>
    [[nodiscard]] constexpr float RadiansToDegrees(const type& radians)
    {
        return radians * (type(180) / std::numbers::pi_v<type>);
    }

    template<class type>
    requires std::is_floating_point_v<type>
    [[nodiscard]] constexpr float DegreesToRadians(const type& degrees)
    {
        return degrees * (std::numbers::pi_v<type> / type(180));
    }

    template<class type>
    requires std::is_floating_point_v<type>
    [[nodiscard]] type HorizontalFovToVertical(const type& horFov, const type& aspect)
    {
        const auto fovRad = DegreesToRadians(horFov);

        const auto vertFov = type(2) * std::atan(std::tan(fovRad / type(2)) / aspect);

        return RadiansToDegrees(vertFov);
    }

    template<class type>
    requires std::is_floating_point_v<type>
    [[nodiscard]] type VerticalFovToHorizontal(const type& vertFov, const type& aspect)
    {
        const auto fovRad = DegreesToRadians(vertFov);

        const auto horFov = type(2) * std::atan(std::tan(fovRad / type(2)) * aspect);

        return RadiansToDegrees(horFov);
    }
}
