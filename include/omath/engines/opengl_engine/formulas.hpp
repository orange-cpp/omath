//
// Created by Orange on 12/23/2024.
//
#pragma once
#include "omath/engines/opengl_engine/constants.hpp"

namespace omath::opengl_engine
{
    [[nodiscard]]
    inline constexpr Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept;

    [[nodiscard]]
    inline constexpr Vector3<float> forward_vector(const ViewAngles& angles) noexcept
    {
        const auto vec =
                rotation_matrix(angles) * mat_column_from_vector<float, MatStoreType::COLUMN_MAJOR>(k_abs_forward);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    [[nodiscard]]
    inline constexpr Vector3<float> right_vector(const ViewAngles& angles) noexcept
    {
        const auto vec =
                rotation_matrix(angles) * mat_column_from_vector<float, MatStoreType::COLUMN_MAJOR>(k_abs_right);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    [[nodiscard]]
    inline constexpr Vector3<float> up_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector<float, MatStoreType::COLUMN_MAJOR>(k_abs_up);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    [[nodiscard]]
    inline constexpr Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept
    {
        return mat_look_at_right_handed(cam_origin, cam_origin + forward_vector(angles), up_vector(angles));
    }

    [[nodiscard]]
    inline constexpr Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept
    {
        return mat_rotation_axis_z<float, MatStoreType::COLUMN_MAJOR>(angles.roll)
               * mat_rotation_axis_y<float, MatStoreType::COLUMN_MAJOR>(angles.yaw)
               * mat_rotation_axis_x<float, MatStoreType::COLUMN_MAJOR>(angles.pitch);
    }

    [[nodiscard]]
    inline constexpr Vector3<float> extract_origin(const Mat4X4& mat) noexcept
    {
        return mat_extract_origin(mat);
    }

    [[nodiscard]]
    inline constexpr Vector3<float> extract_scale(const Mat4X4& mat) noexcept
    {
        return mat_extract_scale(mat);
    }

    [[nodiscard]]
    inline constexpr ViewAngles extract_rotation_angles(const Mat4X4& mat) noexcept
    {
        const auto angles = mat_extract_rotation_zyx(mat);
        return {
                PitchAngle::from_degrees(angles.x),
                YawAngle::from_degrees(angles.y),
                RollAngle::from_degrees(angles.z),
        };
    }

    [[nodiscard]]
    inline constexpr Mat4X4 calc_perspective_projection_matrix(
            const float field_of_view, const float aspect_ratio, const float near_plane, const float far_plane,
            const NDCDepthRange ndc_depth_range = NDCDepthRange::NEGATIVE_ONE_TO_ONE) noexcept
    {
        if (ndc_depth_range == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return mat_perspective_right_handed_vertical_fov<float, MatStoreType::COLUMN_MAJOR,
                                                             NDCDepthRange::NEGATIVE_ONE_TO_ONE>(
                    field_of_view, aspect_ratio, near_plane, far_plane);

        if (ndc_depth_range == NDCDepthRange::ZERO_TO_ONE)
            return mat_perspective_right_handed_vertical_fov<float, MatStoreType::COLUMN_MAJOR,
                                                             NDCDepthRange::ZERO_TO_ONE>(field_of_view, aspect_ratio,
                                                                                         near_plane, far_plane);

        std::unreachable();
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType units_to_centimeters(const FloatingType& units)
    {
        return units / static_cast<FloatingType>(100);
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType units_to_meters(const FloatingType& units)
    {
        return units;
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
        return centimeters * static_cast<FloatingType>(100);
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType meters_to_units(const FloatingType& meters)
    {
        return meters;
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard]]
    constexpr FloatingType kilometers_to_units(const FloatingType& kilometers)
    {
        return meters_to_units(kilometers * static_cast<FloatingType>(1000));
    }
} // namespace omath::opengl_engine
