//
// Created by Vlad on 27.08.2024.
//

#pragma once

#include <expected>
#include <omath/angle.hpp>
#include <omath/mat.hpp>
#include <omath/vector3.hpp>
#include <type_traits>
#include "omath/projection/error_codes.hpp"

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
    using FieldOfView = Angle<float, 0.f, 180.f, AngleFlags::Clamped>;

    template<class Mat4x4Type, class ViewAnglesType>
    class Camera
    {
    public:
        virtual ~Camera() = default;
        Camera(const Vector3<float>& position, const ViewAnglesType& viewAngles, const ViewPort& viewPort,
               const FieldOfView& fov, const float near, const float far) :
            m_viewPort(viewPort), m_fieldOfView(fov), m_farPlaneDistance(far), m_nearPlaneDistance(near),
            m_viewAngles(viewAngles), m_origin(position)
        {
        }

    protected:
        virtual void LookAt(const Vector3<float>& target) = 0;

        [[nodiscard]] virtual Mat4x4Type CalcViewMatrix() const = 0;

        [[nodiscard]] virtual Mat4x4Type CalcProjectionMatrix() const = 0;

        [[nodiscard]] Mat4x4Type CalcViewProjectionMatrix() const
        {
            return CalcProjectionMatrix() * CalcViewMatrix();
        }

    public:
        [[nodiscard]] const Mat4x4Type& GetViewProjectionMatrix() const
        {
            if (!m_viewProjectionMatrix.has_value())
                m_viewProjectionMatrix = CalcViewProjectionMatrix();

            return m_viewProjectionMatrix.value();
        }

        void SetFieldOfView(const FieldOfView& fov)
        {
            m_fieldOfView = fov;
            m_viewProjectionMatrix = std::nullopt;
        }

        void SetNearPlane(const float near)
        {
            m_nearPlaneDistance = near;
            m_viewProjectionMatrix = std::nullopt;
        }

        void SetFarPlane(const float far)
        {
            m_farPlaneDistance = far;
            m_viewProjectionMatrix = std::nullopt;
        }

        void SetViewAngles(const ViewAnglesType& viewAngles)
        {
            m_viewAngles = viewAngles;
            m_viewProjectionMatrix = std::nullopt;
        }

        void SetOrigin(const Vector3<float>& origin)
        {
            m_origin = origin;
            m_viewProjectionMatrix = std::nullopt;
        }

        void SetViewPort(const ViewPort& viewPort)
        {
            m_viewPort = viewPort;
            m_viewProjectionMatrix = std::nullopt;
        }

        [[nodiscard]] const FieldOfView& GetFieldOfView() const
        {
            return m_fieldOfView;
        }

        [[nodiscard]] const float& GetNearPlane() const
        {
            return m_nearPlaneDistance;
        }

        [[nodiscard]] const float& GetFarPlane() const
        {
            return m_farPlaneDistance;
        }

        [[nodiscard]] const ViewAnglesType& GetViewAngles() const
        {
            return m_viewAngles;
        }

        [[nodiscard]] const Vector3<float>& GetOrigin() const
        {
            return m_origin;
        }

        [[nodiscard]] std::expected<Vector3<float>, Error> WorldToScreen(const Vector3<float>& worldPosition) const
        {
            auto normalizedCords = WorldToViewPort(worldPosition);

            if (!normalizedCords.has_value())
                return std::unexpected{normalizedCords.error()};


            return NdcToScreenPosition(*normalizedCords);
        }

        [[nodiscard]] std::expected<Vector3<float>, Error> WorldToViewPort(const Vector3<float>& worldPosition) const
        {
            auto projected = GetViewProjectionMatrix() *
                                   MatColumnFromVector<float, Mat4x4Type::GetStoreOrdering()>(worldPosition);

            if (projected.At(3, 0) == 0.0f)
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

            projected /= projected.At(3, 0);

            if (IsNdcOutOfBounds(projected))
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

            return Vector3<float>{projected.At(0, 0), projected.At(1, 0), projected.At(2, 0)};
        }

    protected:
        ViewPort m_viewPort{};
        Angle<float, 0.f, 180.f, AngleFlags::Clamped> m_fieldOfView;

        mutable std::optional<Mat4x4Type> m_viewProjectionMatrix;

        float m_farPlaneDistance;
        float m_nearPlaneDistance;


        ViewAnglesType m_viewAngles;
        Vector3<float> m_origin;

    private:
        template<class Type>
        [[nodiscard]]
        constexpr static bool IsNdcOutOfBounds(const Type& ndc)
        {
            return std::ranges::any_of(ndc.RawArray(), [](const auto& val) { return val < -1 || val > 1; });
        }

        [[nodiscard]] Vector3<float> NdcToScreenPosition(const Vector3<float>& ndc) const
        {
            return
            {
                (ndc.x + 1.f) / 2.f * m_viewPort.m_width,
                (1.f - ndc.y) / 2.f * m_viewPort.m_height,
                ndc.z
            };
        }
    };
} // namespace omath::projection
