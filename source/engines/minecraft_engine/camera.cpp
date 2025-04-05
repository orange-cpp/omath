//
// Created by Vladislav on 05.04.2025.
//
#include "omath/engines/minecraft_engine/camera.hpp"
#include <source_location>
#include "omath/engines/minecraft_engine/formulas.hpp"

namespace omath::minecraft_engine
{
    void Camera::LookAt([[maybe_unused]] const Vector3<float>& target)
    {
        constexpr std::source_location loc = std::source_location::current();

        throw std::runtime_error(std::format("{} Not implemented!", loc.function_name()));
    }
    Mat4x4 Camera::CalcViewMatrix() const
    {
        return minecraft_engine::CalcViewMatrix(m_viewAngles, m_origin);
    }
    Mat4x4 Camera::CalcProjectionMatrix() const
    {
        return CalcViewMatrix() * CalcPerspectiveProjectionMatrix(m_fieldOfView.AsDegrees(), m_viewPort.AspectRatio(),
            m_nearPlaneDistance, m_farPlaneDistance);
    }
} // namespace omath::minecraft_engine
