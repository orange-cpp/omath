//
// Created by Orange on 12/23/2024.
//
#include "omath/engines/opengl_engine/camera.hpp"
#include "omath/engines/opengl_engine/formulas.hpp"

namespace omath::opengl_engine
{

    Camera::Camera(const Vector3<float>& position, const ViewAngles& view_angles, const projection::ViewPort& view_port,
                   const Angle<float, 0.f, 180.f, AngleFlags::Clamped>& fov, const float near, const float far)
        : projection::Camera<Mat4X4, ViewAngles>(position, view_angles, view_port, fov, near, far)
    {
    }
    void Camera::look_at([[maybe_unused]] const Vector3<float>& target)
    {
        const float distance = m_origin.distance_to(target);
        const auto delta = target - m_origin;

        m_view_angles.pitch = PitchAngle::from_radians(std::asin(delta.z / distance));
        m_view_angles.yaw = -YawAngle::from_radians(std::atan2(delta.y, delta.x));
        m_view_angles.roll = RollAngle::from_radians(0.f);
    }
    Mat4X4 Camera::calc_view_matrix() const
    {
        return opengl_engine::calc_view_matrix(m_view_angles, m_origin);
    }
    Mat4X4 Camera::calc_projection_matrix() const
    {
        return calc_perspective_projection_matrix(m_field_of_view.as_degrees(), m_view_port.AspectRatio(),
                                                  m_near_plane_distance, m_far_plane_distance);
    }
} // namespace omath::opengl_engine
