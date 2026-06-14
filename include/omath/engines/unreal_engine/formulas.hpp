//
// Created by Vlad on 3/22/2025.
//

#pragma once
#include "omath/engines/unreal_engine/constants.hpp"

namespace omath::unreal_engine
{
    [[nodiscard]]
    inline OMATH_CONSTEXPR Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept;

    [[nodiscard]]
    inline OMATH_CONSTEXPR Vector3<double> forward_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_forward);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Vector3<double> right_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_right);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Vector3<double> up_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_up);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Mat4X4 calc_view_matrix(const ViewAngles& angles,
                                                    const Vector3<double>& cam_origin) noexcept
    {
        return mat_camera_view<double, MatStoreType::ROW_MAJOR>(forward_vector(angles), right_vector(angles),
                                                               up_vector(angles), cam_origin);
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept
    {
        return mat_rotation_axis_z<double, MatStoreType::ROW_MAJOR>(angles.yaw)
               * mat_rotation_axis_y<double, MatStoreType::ROW_MAJOR>(-angles.pitch)
               * mat_rotation_axis_x<double, MatStoreType::ROW_MAJOR>(-angles.roll);
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Vector3<double> extract_origin(const Mat4X4& mat) noexcept
    {
        return mat_extract_origin(mat);
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Vector3<double> extract_scale(const Mat4X4& mat) noexcept
    {
        return mat_extract_scale(mat);
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR ViewAngles extract_rotation_angles(const Mat4X4& mat) noexcept
    {
        const auto angles = mat_extract_rotation_zyx(mat);
        return {
                PitchAngle::from_degrees(-angles.y),
                YawAngle::from_degrees(angles.z),
                RollAngle::from_degrees(-angles.x),
        };
    }

    [[nodiscard]]
    inline OMATH_CONSTEXPR Mat4X4 calc_perspective_projection_matrix(
            const double field_of_view, const double aspect_ratio, const double near_plane, const double far_plane,
            const NDCDepthRange ndc_depth_range = NDCDepthRange::NEGATIVE_ONE_TO_ONE) noexcept
    {
        if (ndc_depth_range == NDCDepthRange::ZERO_TO_ONE)
            return mat_perspective_left_handed_horizontal_fov<
                    double, MatStoreType::ROW_MAJOR, NDCDepthRange::ZERO_TO_ONE>(
                    field_of_view, aspect_ratio, near_plane, far_plane);
        if (ndc_depth_range == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return mat_perspective_left_handed_horizontal_fov<
                    double, MatStoreType::ROW_MAJOR, NDCDepthRange::NEGATIVE_ONE_TO_ONE>(
                    field_of_view, aspect_ratio, near_plane, far_plane);
        std::unreachable();
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType units_to_centimeters(const FloatingType& units)
    {
        return units;
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType units_to_meters(const FloatingType& units)
    {
        return units / static_cast<FloatingType>(100);
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
        return centimeters;
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType meters_to_units(const FloatingType& meters)
    {
        return meters * static_cast<FloatingType>(100);
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType kilometers_to_units(const FloatingType& kilometers)
    {
        return meters_to_units(kilometers * static_cast<FloatingType>(1000));
    }
} // namespace omath::unreal_engine
