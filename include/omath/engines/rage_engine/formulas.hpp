//
// Created by Orange on 6/3/2026.
//

#pragma once
#include "omath/engines/rage_engine/constants.hpp"
#include <type_traits>

namespace omath::rage_engine
{
    [[nodiscard("rotation matrix result should not be discarded")]]
    constexpr Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept;

    [[nodiscard("forward vector result should not be discarded")]]
    constexpr Vector3<float> forward_vector(const ViewAngles& angles) noexcept
    {
        return mat_rotate_vector(rotation_matrix(angles), k_abs_forward);
    }

    [[nodiscard("right vector result should not be discarded")]]
    constexpr Vector3<float> right_vector(const ViewAngles& angles) noexcept
    {
        return mat_rotate_vector(rotation_matrix(angles), k_abs_right);
    }

    [[nodiscard("up vector result should not be discarded")]]
    constexpr Vector3<float> up_vector(const ViewAngles& angles) noexcept
    {
        return mat_rotate_vector(rotation_matrix(angles), k_abs_up);
    }

    [[nodiscard("view matrix result should not be discarded")]]
    constexpr Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept
    {
        // Build the rotation once - calling forward/right/up_vector() separately would redo two 4x4 multiplies and six
        // sin/cos evaluations three times over
        const auto rotation = rotation_matrix(angles);

        return mat_camera_view<float, MatStoreType::ROW_MAJOR>(mat_rotate_vector(rotation, k_abs_forward),
                                                               mat_rotate_vector(rotation, k_abs_right),
                                                               mat_rotate_vector(rotation, k_abs_up), cam_origin);
    }

    [[nodiscard("rotation matrix result should not be discarded")]]
    constexpr Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept
    {
        return mat_rotation_zyx<float, MatStoreType::ROW_MAJOR>(angles.yaw, angles.roll, angles.pitch);
    }

    [[nodiscard("origin result should not be discarded")]]
    constexpr Vector3<float> extract_origin(const Mat4X4& mat) noexcept
    {
        return mat_extract_origin(mat);
    }

    [[nodiscard("scale result should not be discarded")]]
    constexpr Vector3<float> extract_scale(const Mat4X4& mat) noexcept
    {
        return mat_extract_scale(mat);
    }

    [[nodiscard("rotation angles result should not be discarded")]]
    constexpr ViewAngles extract_rotation_angles(const Mat4X4& mat) noexcept
    {
        const auto angles = mat_extract_rotation_zyx(mat);
        return {
                PitchAngle::from_degrees(angles.x),
                YawAngle::from_degrees(angles.z),
                RollAngle::from_degrees(angles.y),
        };
    }

    [[nodiscard("perspective projection matrix result should not be discarded")]]
    constexpr Mat4X4
    calc_perspective_projection_matrix(const float field_of_view, const float aspect_ratio, const float near_plane,
                                       const float far_plane,
                                       const NDCDepthRange ndc_depth_range = NDCDepthRange::ZERO_TO_ONE) noexcept
    {
        if (ndc_depth_range == NDCDepthRange::ZERO_TO_ONE)
            return mat_perspective_left_handed_vertical_fov<float, MatStoreType::ROW_MAJOR, NDCDepthRange::ZERO_TO_ONE>(
                    field_of_view, aspect_ratio, near_plane, far_plane);

        if (ndc_depth_range == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return mat_perspective_left_handed_vertical_fov<float, MatStoreType::ROW_MAJOR,
                                                            NDCDepthRange::NEGATIVE_ONE_TO_ONE>(
                    field_of_view, aspect_ratio, near_plane, far_plane);
        std::unreachable();
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard("centimeters value should not be discarded")]]
    constexpr FloatingType units_to_centimeters(const FloatingType& units)
    {
        return units / static_cast<FloatingType>(100);
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard("meters value should not be discarded")]]
    constexpr FloatingType units_to_meters(const FloatingType& units)
    {
        return units;
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard("kilometers value should not be discarded")]]
    constexpr FloatingType units_to_kilometers(const FloatingType& units)
    {
        return units_to_meters(units) / static_cast<FloatingType>(1000);
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard("units value should not be discarded")]]
    constexpr FloatingType centimeters_to_units(const FloatingType& centimeters)
    {
        return centimeters * static_cast<FloatingType>(100);
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard("units value should not be discarded")]]
    constexpr FloatingType meters_to_units(const FloatingType& meters)
    {
        return meters;
    }

    template<class FloatingType>
    requires std::is_floating_point_v<FloatingType>
    [[nodiscard("units value should not be discarded")]]
    constexpr FloatingType kilometers_to_units(const FloatingType& kilometers)
    {
        return meters_to_units(kilometers * static_cast<FloatingType>(1000));
    }
} // namespace omath::rage_engine
