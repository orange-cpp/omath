//
// Created by Vlad on 3/22/2025.
//
#include <omath/engines/unity_engine/camera.hpp>
#include <omath/engines/unity_engine/formulas.hpp>

namespace omath::unity_engine
{
    Camera::Camera(const Vector3<float>& position, const ViewAngles& view_angles, const projection::ViewPort& view_port,
                   const projection::FieldOfView& fov, const float near, const float far)
        : projection::Camera<Mat4X4, ViewAngles>(position, view_angles, view_port, fov, near, far)
    {
    }
    void Camera::look_at([[maybe_unused]] const Vector3<float>& target)
    {
        throw std::runtime_error("Not implemented");
    }
    Mat4X4 Camera::calc_view_matrix() const
    {
        return unity_engine::calc_view_matrix(m_view_angles, m_origin);
    }
    Mat4X4 Camera::calc_projection_matrix() const
    {
        return calc_perspective_projection_matrix(m_field_of_view.as_degrees(), m_view_port.aspect_ratio(),
                                                  m_near_plane_distance, m_far_plane_distance);
    }
} // namespace omath::unity_engine
