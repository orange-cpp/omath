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
    using FieldOfView = const Angle<float, 0.f, 180.f, AngleFlags::Clamped>;

    template<class Mat4x4Type, class ViewAnglesType>
    class Camera
    {

    public:
        virtual ~Camera() = default;
        Camera(const Vector3& position, const ViewAnglesType& viewAngles, const ViewPort& viewPort,
               const FieldOfView& fov, const float near, const float far) :
            m_viewPort(viewPort), m_fieldOfView(fov), m_farPlaneDistance(far), m_nearPlaneDistance(near),
            m_viewAngles(viewAngles), m_origin(position)
        {

        }

        virtual void LookAt(const Vector3& target) = 0;

        [[nodiscard]] virtual Mat4x4Type GetViewMatrix() const = 0;

        [[nodiscard]] virtual Mat4x4Type GetProjectionMatrix() const = 0;

        [[nodiscard]] Mat4x4Type GetViewProjectionMatrix()
        {
            return GetProjectionMatrix() * GetViewMatrix();
        }

        [[nodiscard]] std::expected<Vector3, Error> WorldToScreen(const Mat4x4Type& viewProj, const Vector3& worldPosition) const
        {
            auto projected = viewProj * MatColumnFromVector<float, Mat4x4Type::GetStoreOrdering()>(worldPosition);

            if (projected.At(3, 0) == 0.0f)
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

            projected /= projected.At(3, 0);

            if (IsNdcOutOfBounds(projected))
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

            return Vector3{(projected.At(0,0)+1) / 2 * m_viewPort.m_width , (-projected.At(1,0)+1) / 2 * m_viewPort.m_height, projected.At(2,0)};
        }

    protected:
        ViewPort m_viewPort{};
        Angle<float, 0.f, 180.f, AngleFlags::Clamped> m_fieldOfView;

        float m_farPlaneDistance;
        float m_nearPlaneDistance;


        ViewAnglesType m_viewAngles;
        Vector3 m_origin;

    private:
        template<class Type>
        [[nodiscard]]
        constexpr static bool IsNdcOutOfBounds(const Type& ndc)
        {
            return std::ranges::any_of( ndc.RawArray(), [](const auto& val) { return val < -1 || val > 1; });
        }
    };
} // namespace omath::projection
