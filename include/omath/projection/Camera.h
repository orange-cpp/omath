//
// Created by Vlad on 27.08.2024.
//

#pragma once

#include <expected>
#include <omath/Vector3.h>
#include <omath/Matrix.h>
#include <string_view>
#include "ErrorCodes.h"


namespace omath::projection
{
    class ViewPort final
    {
    public:
        float m_width;
        float m_height;

        [[nodiscard]] constexpr float AspectRatio() const {return  m_width / m_height;}
    };

    class Camera
    {
    public:
        Camera(const Vector3& position, const Vector3& viewAngles, const ViewPort& viewPort, float fov, float near, float far);
        void SetViewAngles(const Vector3& viewAngles);

        [[nodiscard]] Matrix GetViewMatrix() const;

        [[nodiscard]] std::expected<Vector3, Error> WorldToScreen(const Vector3& worldPosition) const;

        ViewPort m_viewPort{};
        float m_fieldOfView;

        float m_farPlaneDistance;
        float m_nearPlaneDistance;

    private:
        Vector3 m_viewAngles;
        Vector3 m_origin;
    };
}
