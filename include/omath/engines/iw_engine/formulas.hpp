//
// Created by Vlad on 3/17/2025.
//

#pragma once
#include "omath/engines/iw_engine/constants.hpp"

namespace omath::iw_engine
{
    [[nodiscard]]
    Vector3<float> forward_vector(const ViewAngles& angles) noexcept;

    [[nodiscard]]
    Vector3<float> right_vector(const ViewAngles& angles) noexcept;

    [[nodiscard]]
    Vector3<float> up_vector(const ViewAngles& angles) noexcept;

    [[nodiscard]]
    Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept;

    [[nodiscard]] Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept;

    [[nodiscard]]
    Mat4X4 calc_perspective_projection_matrix(float field_of_view, float aspect_ratio, float near, float far) noexcept;

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType units_to_centimeters(const FloatingType& units)
    {
        constexpr auto centimeter_in_unit = static_cast<FloatingType>(2.54);
        return units * centimeter_in_unit;
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType units_to_meters(const FloatingType& units)
    {
        return units_to_centimeters(units) / static_cast<FloatingType>(100);
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType units_to_kilometers(const FloatingType& units)
    {
        return units_to_meters(units) / static_cast<FloatingType>(1000);
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType centimeters_to_units(const FloatingType& centimeters)
    {
        constexpr auto centimeter_in_unit = static_cast<FloatingType>(2.54);
        return centimeters / centimeter_in_unit;
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType meters_to_units(const FloatingType& meters)
    {
        return centimeters_to_units(meters * static_cast<FloatingType>(100));
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType kilometers_to_units(const FloatingType& kilometers)
    {
        return meters_to_units(kilometers * static_cast<FloatingType>(1000));
    }
} // namespace omath::iw_engine
