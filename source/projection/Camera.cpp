//
// Created by Vlad on 27.08.2024.
//
#include "omath/projection/Camera.h"

#include <complex>

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

    float & Camera::GetFloat1() {
        static float m_float1 = 1.52550f;
        return m_float1;
    }

    float & Camera::GetFloat2() {
        static float m_float2 = 1.14500f;
        return m_float2;
    }

    Matrix Camera::GetViewMatrix() const
    {
        return GetTranslationMatrix() * GetOrientationMatrix();
    }

    Matrix Camera::GetProjectionMatrix(const float scaleX, const float scaleY) const
    {
        const float fRight = std::tan(angles::DegreesToRadians(m_fieldOfView) / 2.f);
        const float fLeft  = -fRight;

        const float verticalFov = angles::DegreesToRadians(m_fieldOfView) * (m_viewPort.y / m_viewPort.x);

        const float top   = std::tan(verticalFov / 2.f);
        const float botton = -top;

        const auto far = m_farPlaneDistance;
        const auto near = m_nearPlaneDistance;

        return Matrix({
            {scaleX / (fRight - fLeft), 0.f, 0.f, 0.f},
            {0.f, scaleY / (top - botton), 0.f, 0.f},
            {0.f, 0.f, (far + near) / (far - near), 1.f},
            {0.f, 0.f, -near * far / (far - near), 0.f},
        });
    }

    Matrix Camera::GetTranslationMatrix() const
    {
        return Matrix(
        {
            {1.f, 0.f, 0.f, 0.f},
            {0.f, 1.f, 0.f, 0.f},
            {0.f, 0.f, 1.f, 0.f},
            {-m_origin.x, -m_origin.y, -m_origin.z, 1.f},
        });
    }

    Matrix Camera::GetOrientationMatrix() const
    {
        const auto forward = Vector3::ForwardVector(m_viewAngles.x, m_viewAngles.y);
        const auto right = Vector3::RightVector(m_viewAngles.x, m_viewAngles.y, m_viewAngles.z);
        const auto up = Vector3::UpVector(m_viewAngles.x, m_viewAngles.y, m_viewAngles.z);


        return Matrix(
        {
            {right.x, up.x, forward.x, 0.f},
            {right.y, up.y, forward.y, 0.f},
            {right.z, up.z, forward.z, 0.f},
            {0.f,      0.f,       0.f, 1.f},
        });
    }

    std::expected<Vector3, std::string_view> Camera::WorldToScreen(const Vector3 &worldPosition) const
    {
        const auto posVecAsMatrix = Matrix({{worldPosition.x, worldPosition.y, worldPosition.z, 1.f}});

        const auto viewProjectionMatrix = GetViewMatrix() * GetProjectionMatrix(GetFloat1(), GetFloat2());

        auto projected = posVecAsMatrix * viewProjectionMatrix;

        if (projected.At(0, 3) <= 0.f)
            return std::unexpected("Projection point is out of camera field of view");

        projected /= projected.At(0, 3);

        projected *= Matrix::ToScreenMatrix(m_viewPort.x, m_viewPort.y);

        return Vector3{projected.At(0, 0), projected.At(0, 1), projected.At(0, 2)};
    }
}
