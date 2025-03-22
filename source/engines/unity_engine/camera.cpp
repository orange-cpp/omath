//
// Created by Vlad on 3/22/2025.
//
#include <omath/engines/unity_engine/camera.hpp>


namespace omath::unity_engine
{
    Camera::Camera(const Vector3<float>& position, const ViewAngles& viewAngles, const projection::ViewPort& viewPort,
                   const projection::FieldOfView& fov, const float near, const float far) :
        projection::Camera<Mat4x4, ViewAngles>(position, viewAngles, viewPort, fov, near, far)
    {
    }
    void Camera::LookAt([[maybe_unused]] const Vector3<float>& target)
    {
        throw std::runtime_error("Not implemented");
    }
    Mat4x4 Camera::CalcViewMatrix() const
    {
        return unity_engine::CalcViewMatrix(m_viewAngles, m_origin);
    }
    Mat4x4 Camera::CalcProjectionMatrix() const
    {
        return CalcPerspectiveProjectionMatrix(m_fieldOfView.AsDegrees(), m_viewPort.AspectRatio(), m_nearPlaneDistance,
                                               m_farPlaneDistance);
    }
} // namespace omath::unity_engine
