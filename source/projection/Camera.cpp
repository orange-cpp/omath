//
// Created by Vlad on 27.08.2024.
//
#include "omath/projection/Camera.h"

#include <complex>

#include "omath/Angles.h"


namespace omath::projection
{
    Camera::Camera(const Vector3 &position, const Vector3 &viewAngles, const ViewPort &viewPort,
                   const float fov, const float near, const float far)
    {
        m_origin = position;
        m_viewAngles = viewAngles;
        m_viewPort = viewPort;
        m_fieldOfView = fov;
        m_nearPlaneDistance = near;
        m_farPlaneDistance = far;
    }

    Mat<4, 4> Camera::GetViewMatrix() const
    {
        const auto forward = Vector3::ForwardVector(m_viewAngles.x, m_viewAngles.y);
        const auto right = Vector3::RightVector(m_viewAngles.x, m_viewAngles.y, m_viewAngles.z);
        const auto up = Vector3::UpVector(m_viewAngles.x, m_viewAngles.y, m_viewAngles.z);

        return Mat<4, 4>::TranslationMat(-m_origin) * Mat<4, 4>::OrientationMat(forward, right, up);
    }

    std::expected<Vector2, Error> Camera::WorldToScreen(const Vector3 &worldPosition) const
    {
        const auto posVecAsMatrix = Mat<1, 4>({{worldPosition.x, worldPosition.y, worldPosition.z, 1.f}});


        const auto projectionMatrix = Mat<4, 4>::ProjectionMat(m_fieldOfView, m_viewPort.AspectRatio(),
                                                               m_nearPlaneDistance, m_farPlaneDistance);

        Mat<1, 4> projected = posVecAsMatrix * (GetViewMatrix() * projectionMatrix);

        if (projected.At(0, 3) <= 0.f)
            return std::unexpected(Error::WORLD_POSITION_IS_BEHIND_CAMERA);

        projected /= projected.At(0, 3);

        if (projected.At(0, 0) < -1.f || projected.At(0, 0) > 1.f ||
            projected.At(0, 1) < -1.f || projected.At(0, 1) > 1.f)
            return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

        projected *= Mat<4, 4>::ToScreenMat(m_viewPort.m_width, m_viewPort.m_height);

        return Vector2{projected.At(0, 0), projected.At(0, 1)};
    }
}
