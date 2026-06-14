//
// Created by Vlad on 3/17/2025.
//

#pragma once
#include "omath/engines/iw_engine/constants.hpp"

namespace omath::iw_engine
{
    [[nodiscard]]
    inline OMATH_CONSTEXPR Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept;

    [[nodiscard]]
    inline OMATH_CONSTEXPR Vector3<float> forward_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_forward);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Vector3<float> right_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_right);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Vector3<float> up_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_up);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept
    {
        return mat_rotation_axis_z(angles.yaw) * mat_rotation_axis_y(angles.pitch) * mat_rotation_axis_x(angles.roll);
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Vector3<float> extract_origin(const Mat4X4& mat) noexcept
    {
        return mat_extract_origin(mat);
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Vector3<float> extract_scale(const Mat4X4& mat) noexcept
    {
        return mat_extract_scale(mat);
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR ViewAngles extract_rotation_angles(const Mat4X4& mat) noexcept
    {
        const auto angles = mat_extract_rotation_zyx(mat);
        return {
                PitchAngle::from_degrees(angles.y),
                YawAngle::from_degrees(angles.z),
                RollAngle::from_degrees(angles.x),
        };
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Mat4X4 calc_view_matrix(const ViewAngles& angles,
                                                   const Vector3<float>& cam_origin) noexcept
    {
        return mat_camera_view(forward_vector(angles), right_vector(angles), up_vector(angles), cam_origin);
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Mat4X4 calc_perspective_projection_matrix(
            const float field_of_view, const float aspect_ratio, const float near_plane, const float far_plane,
            const NDCDepthRange ndc_depth_range = NDCDepthRange::NEGATIVE_ONE_TO_ONE) noexcept
    {
        constexpr float k_source_reference_aspect = 4.f / 3.f;
        const auto vertical_fov = angles::horizontal_fov_to_vertical(field_of_view, k_source_reference_aspect);

        if (ndc_depth_range == NDCDepthRange::ZERO_TO_ONE)
            return mat_perspective_left_handed_vertical_fov<float, MatStoreType::ROW_MAJOR, NDCDepthRange::ZERO_TO_ONE>(
                    vertical_fov, aspect_ratio, near_plane, far_plane);
        if (ndc_depth_range == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return mat_perspective_left_handed_vertical_fov<float, MatStoreType::ROW_MAJOR,
                                                            NDCDepthRange::NEGATIVE_ONE_TO_ONE>(
                    vertical_fov, aspect_ratio, near_plane, far_plane);
        std::unreachable();
    }

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
