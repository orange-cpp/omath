//
// Created by Vlad on 27.08.2024.
//

#pragma once

#include <expected>
#include <omath/Vector3.h>
#include <omath/Matrix.h>
#include <string_view>


namespace omath::projection
{
    class Camera
    {
    public:
        Camera(const Vector3& position, const Vector3& viewAngles, const Vector3& viewPort, float fov, float near, float far);
        void SetViewAngles(const Vector3& viewAngles);
        [[nodiscard]] const Vector3& GetViewAngles() const;

        [[nodiscard]] Matrix GetViewMatrix() const;
        [[nodiscard]] Matrix GetProjectionMatrix() const;
        [[nodiscard]] Matrix GetTranslationMatrix() const;
        [[nodiscard]] Matrix GetOrientationMatrix() const;

        [[nodiscard]] std::expected<Vector3, std::string_view> WorldToScreen(const Vector3& worldPosition) const;

        Vector3 m_viewPort;
        float m_fieldOfView;

        float m_farPlaneDistance;
        float m_nearPlaneDistance;

    private:
        Vector3 m_viewAngles;
        Vector3 m_origin;
    };
}
