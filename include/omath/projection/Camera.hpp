//
// Created by Vlad on 27.08.2024.
//

#pragma once

#include <expected>
#include <omath/Mat.hpp>
#include <omath/Vector3.hpp>
#include "ErrorCodes.hpp"
#include <omath/Angle.hpp>
#include <type_traits>


namespace omath::projection
{
    class ViewPort final
    {
    public:
        float m_width;
        float m_height;

        [[nodiscard]] constexpr float AspectRatio() const
        {
            return m_width / m_height;
        }
    };

    template<class ViewAnglesType, class ViewMatFunc, class ProjectionFunc>
    requires std::is_same_v<std::invoke_result_t<ViewMatFunc, const ViewAnglesType&, const Vector3&>,
        std::invoke_result_t<ProjectionFunc, const float&, const float&, const float&, const float&>>
    class Camera
    {
    public:
        Camera(const Vector3& position, const ViewAnglesType& viewAngles, const ViewPort& viewPort,
               const Angle<float, 0.f, 180.f, AngleFlags::Clamped>& fov, const float near, const float far,
               const std::function<ViewMatFunc>& viewMatFunc, const std::function<ProjectionFunc>& projFunc) :
            m_viewPort(viewPort), m_fieldOfView(fov), m_farPlaneDistance(far), m_nearPlaneDistance(near),
            m_viewAngles(viewAngles), m_origin(position), CreateViewMatrix(viewMatFunc), CreateProjectionMatrix(projFunc)
        {
        }

        void LookAt(const Vector3& target);

        [[nodiscard]] auto GetViewMatrix() const
        {
            return CreateViewMatrix(m_viewAngles, m_origin);
        }

        [[nodiscard]] auto GetProjectionMatrix() const
        {
            return CreateProjectionMatrix(m_fieldOfView.AsDegrees(), m_viewPort.AspectRatio(), m_nearPlaneDistance, m_farPlaneDistance);
        }

        [[nodiscard]] auto GetViewProjectionMatrix() const
        {
            return GetProjectionMatrix() * GetViewMatrix();
        }
        [[nodiscard]] std::expected<Vector3, Error> WorldToScreen([[maybe_unused]] const Vector3& worldPosition) const
        {
            using mat = std::invoke_result_t<ViewMatFunc, const ViewAnglesType&, const Vector3&>;
            Mat<4, 1> projected = GetViewProjectionMatrix() * MatColumnFromVector<float, mat::GetStoreOrdering()>(worldPosition);

            if (projected.At(3, 0) == 0.0f)
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);
            projected /= projected.At(3, 0);

            return Vector3{++projected.At(0,0) / 2 * m_viewPort.m_width , ++projected.At(1,0) / 2 * m_viewPort.m_height, projected.At(2,0)};
        }

        ViewPort m_viewPort{};
        Angle<float, 0.f, 180.f, AngleFlags::Clamped> m_fieldOfView;

        float m_farPlaneDistance;
        float m_nearPlaneDistance;

    private:
        ViewAnglesType m_viewAngles;
        Vector3 m_origin;

        std::function<ViewMatFunc> CreateViewMatrix;
        std::function<ProjectionFunc> CreateProjectionMatrix;
    };
} // namespace omath::projection
