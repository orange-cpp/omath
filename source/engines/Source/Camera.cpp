//
// Created by Orange on 12/4/2024.
//
#include "omath/engines/Source/Camera.hpp"
#include "omath/engines/Source/Formulas.hpp"


namespace omath::source
{

    void Camera::LookAt(const Vector3& target)
    {
        const float distance = m_origin.DistTo(target);
        const auto delta = target - m_origin;


        m_viewAngles.pitch = PitchAngle::FromRadians(std::asin(delta.z / distance));
        m_viewAngles.yaw = YawAngle::FromRadians(std::atan2(delta.y, delta.x));
        m_viewAngles.roll = RollAngle::FromRadians(0.f);
    }

    Mat4x4 Camera::GetViewMatrix() const
    {
        return ViewMatrix(m_viewAngles, m_origin);
    }

    Mat4x4 Camera::GetProjectionMatrix() const
    {
        return PerspectiveProjectionMatrix(m_fieldOfView.AsDegrees(), m_viewPort.AspectRatio(), m_nearPlaneDistance, m_farPlaneDistance);
    }
} // namespace omath::source
