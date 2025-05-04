//
// Created by Orange on 12/4/2024.
//
#include "omath/engines/source_engine/camera.hpp"
#include "omath/engines/source_engine/formulas.hpp"

namespace omath::source_engine
{

    Camera::Camera(const Vector3<float>& position, const ViewAngles& view_angles, const projection::ViewPort& view_port,
                   const projection::FieldOfView& fov, const float near, const float far)
        : projection::Camera<Mat4X4, ViewAngles>(position, view_angles, view_port, fov, near, far)
    {
    }
    void Camera::look_at(const Vector3<float>& target)
    {
        const float distance = m_origin.distance_to(target);
        const auto delta = target - m_origin;

        m_view_angles.pitch = PitchAngle::from_radians(std::asin(delta.z / distance));
        m_view_angles.yaw = -YawAngle::from_radians(std::atan2(delta.y, delta.x));
        m_view_angles.roll = RollAngle::from_radians(0.f);
    }

    Mat4X4 Camera::calc_view_matrix() const noexcept
    {
        return source_engine::calc_view_matrix(m_view_angles, m_origin);
    }

    Mat4X4 Camera::calc_projection_matrix() const noexcept
    {
        return calc_perspective_projection_matrix(m_field_of_view.as_degrees(), m_view_port.aspect_ratio(),
                                                  m_near_plane_distance, m_far_plane_distance);
    }
} // namespace omath::source_engine
