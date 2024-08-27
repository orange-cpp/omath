//
// Created by Vlad on 27.08.2024.
//
#include "omath/projection/Camera.h"
#include "omath/angles.h"


namespace omath::projection
{
    Camera::Camera(const Vector3 &position, const Vector3 &viewAngles, const Vector3 &viewPort, const float fov, const float near,
        const float far)
    {
        m_origin = position;
        m_viewAngles = viewAngles;
        m_viewPort = viewPort;
        m_fieldOfView = fov;
        m_nearPlaneDistance = near;
        m_farPlaneDistance = far;
    }

    Matrix Camera::GetViewMatrix() const
    {
        return GetOrientationMatrix() * GetTranslationMatrix();
    }

    Matrix Camera::GetProjectionMatrix() const
    {
        const auto aspectRatio = m_viewPort.x / m_viewPort.y;
        const auto verticalFov = std::tan(angles::DegreesToRadians(m_fieldOfView) / 2.f);


        return Matrix(
            {
                {1.f / (aspectRatio * verticalFov), 0.f, 0.f, 0.f},
                {0.f, 1.f / verticalFov, 0.f, 0.f},
                {0.f, 0.f, m_farPlaneDistance / (m_farPlaneDistance-m_nearPlaneDistance), -m_farPlaneDistance*m_nearPlaneDistance / (m_farPlaneDistance-m_nearPlaneDistance)},
                {0.f, 0.f, 1.f, 0.f}
            });
    }

    Matrix Camera::GetTranslationMatrix() const
    {
        return Matrix(
        {
            {1.f, 0.f, 0.f, m_origin.x},
            {0.f, 1.f, 0.f, m_origin.y},
            {0.f, 0.f, 1.f, m_origin.z},
            {0.f, 0.f, 0.f, 1.f},
        });
    }

    Matrix Camera::GetOrientationMatrix() const
    {
        const auto forward = Vector3::ForwardVector(m_viewAngles.x, m_viewAngles.y);
        const auto right = Vector3::RightVector(m_viewAngles.x, m_viewAngles.y, m_viewAngles.z);
        const auto up = Vector3::UpVector(m_viewAngles.x, m_viewAngles.y, m_viewAngles.z);


        return Matrix(
        {
            {right.x,    up.y,        right.z,          0.f},
            {up.x,       up.y,        up.y,             0.f},
            {forward.x,  forward.y,   forward.z,        0.f},
            {0.f,        0.f,         0.f,              1.f},
        });
    }

    std::expected<Vector3, std::string_view> Camera::WorldToScreen(const Vector3 &worldPosition) const
    {
        const auto posVecAsMatrix = Matrix({{worldPosition.x, worldPosition.y, worldPosition.z, 1.f}});

        const auto viewProjectionMatrix = GetViewMatrix() * GetProjectionMatrix();

        auto projected = posVecAsMatrix * viewProjectionMatrix;

        if (projected.At(0, 3) <= 0.f)
            return std::unexpected("Projection point is out of camera field of view");

        projected /= projected.At(0, 3);

        projected *= Matrix::ToScreenMatrix(m_viewPort.x, m_viewPort.y);

        return Vector3{projected.At(0, 0), projected.At(0, 1), projected.At(0, 2)};
    }
}
