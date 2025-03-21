//
// Created by Orange on 12/23/2024.
//
#include "omath/engines/opengl_engine/Camera.hpp"
#include "omath/engines/opengl_engine/Formulas.hpp"


namespace omath::opengl_engine
{

    Camera::Camera(const Vector3<float>& position, const ViewAngles& viewAngles, const projection::ViewPort& viewPort,
                   const Angle<float, 0.f, 180.f, AngleFlags::Clamped>& fov, const float near, const float far) :
        projection::Camera<Mat4x4, ViewAngles>(position, viewAngles, viewPort, fov, near, far)
    {
    }
    void Camera::LookAt([[maybe_unused]] const Vector3<float>& target)
    {
        const float distance = m_origin.DistTo(target);
        const auto delta = target - m_origin;


        m_viewAngles.pitch = PitchAngle::FromRadians(std::asin(delta.z / distance));
        m_viewAngles.yaw = -YawAngle::FromRadians(std::atan2(delta.y, delta.x));
        m_viewAngles.roll = RollAngle::FromRadians(0.f);
    }
    Mat4x4 Camera::CalcViewMatrix() const
    {
        return opengl_engine::CalcViewMatrix(m_viewAngles, m_origin);
    }
    Mat4x4 Camera::CalcProjectionMatrix() const
    {
        return CalcPerspectiveProjectionMatrix(m_fieldOfView.AsDegrees(), m_viewPort.AspectRatio(), m_nearPlaneDistance,
                                               m_farPlaneDistance);
    }
} // namespace omath::opengl
