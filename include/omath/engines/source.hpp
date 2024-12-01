//
// Created by Orange on 11/24/2024.
//
#pragma once
#include "omath/Mat.hpp"
#include "omath/Vector3.hpp"

#include <omath/Angle.hpp>
#include <omath/ViewAngles.hpp>
#include <omath/projection/Camera.hpp>

namespace omath::source
{
    constexpr Vector3 kAbsUp = {0, 0, 1};
    constexpr Vector3 kAbsRight = {0, -1, 0};
    constexpr Vector3 kAbsForward = {1, 0, 0};

    using PitchAngle = Angle<float, -89.f, 89.f, AngleFlags::Clamped>;
    using YawAngle = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;
    using RollAngle = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;

    using ViewAngles = omath::ViewAngles<PitchAngle, YawAngle, RollAngle>;


    inline Vector3 ForwardVector(const ViewAngles& angles);
    inline Vector3 RightVector(const ViewAngles& angles);
    inline Vector3 UpVector(const ViewAngles& angles);


    template<class Type = float>
    requires std::is_floating_point_v<Type> || std::is_integral_v<Type>
    [[nodiscard]] constexpr Mat<4, 4, Type, MatStoreType::ROW_MAJOR> ViewMatrixFromVecs(const Vector3& forward, const Vector3& right,
                                                                         const Vector3& up, const Vector3& camera_pos)
    {
        return MatTranslation<float, MatStoreType::ROW_MAJOR>(-camera_pos) * Mat<4, 4, Type, MatStoreType::ROW_MAJOR>{
                {right.x, up.x, forward.x, 0},
                {right.y, up.y, forward.y, 0},
                {right.z, up.z, forward.z, 0},
                {0, 0, 0, 1},
        };
    }


    template<class Type = float>
    requires std::is_floating_point_v<Type> || std::is_integral_v<Type>
    [[nodiscard]] Mat<4, 4, Type, MatStoreType::ROW_MAJOR> ViewMatrix(const ViewAngles& angles, const Vector3& cam_origin)
    {
        return ViewMatrixFromVecs(ForwardVector(angles), RightVector(angles), UpVector(angles), cam_origin);
    }

    template<class Type>
    requires std::is_floating_point_v<Type> || std::is_integral_v<Type>
    [[nodiscard]] Mat<4, 4, Type, MatStoreType::ROW_MAJOR>
    PerspectiveProjectionMatrix(const Type& fieldOfView, const Type& aspectRatio, const Type& near, const Type& far)
    {
        const float fovHalfTan = std::tan(angles::DegreesToRadians(fieldOfView) / 2);

        return {
                {static_cast<Type>(1) / (aspectRatio * fovHalfTan), 0, 0, 0},
                {0, static_cast<Type>(1) / (fovHalfTan), 0, 0},
                {0, 0, (far + near) / (far - near), -(static_cast<Type>(2) * far * near) / (far - near)},
                {0, 0, 1, 0},
        };
    }
    // Copied from
    // https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/mp/src/mathlib/mathlib_base.cpp#L919
    [[nodiscard]]
    inline Vector3 ForwardVector(const ViewAngles& angles)
    {
        const auto cosPitch = angles.pitch.Cos();
        const auto sinPitch = angles.pitch.Sin();

        const auto cosYaw = angles.yaw.Cos();
        const auto sinYaw = angles.yaw.Sin();


        return {cosPitch * cosYaw, cosPitch * sinYaw, -sinPitch};
    }

    [[nodiscard]]
    inline Vector3 RightVector(const ViewAngles& angles)
    {
        const auto cosPitch = angles.pitch.Cos();
        const auto sinPitch = angles.pitch.Sin();

        const auto cosYaw = angles.yaw.Cos();
        const auto sinYaw = angles.yaw.Sin();

        const auto cosRoll = angles.roll.Cos();
        const auto sinRoll = angles.roll.Sin();


        return
        {
            -1 * sinRoll * sinPitch * cosYaw + -1 * cosRoll * -sinYaw,
            -1 * sinRoll * sinPitch * sinYaw + -1 * cosRoll * cosYaw,
            -1 * sinRoll * cosPitch
        };
    }

    [[nodiscard]]
    inline Vector3 UpVector(const ViewAngles& angles)
    {
        const auto cosPitch = angles.pitch.Cos();
        const auto sinPitch = angles.pitch.Sin();

        const auto cosYaw = angles.yaw.Cos();
        const auto sinYaw = angles.yaw.Sin();

        const auto cosRoll = angles.roll.Cos();
        const auto sinRoll = angles.roll.Sin();


        return
        {
            cosRoll * sinPitch * cosYaw + - sinRoll * -sinYaw,
            cosRoll * sinPitch * sinYaw + - sinRoll * cosYaw,
            cosRoll * cosPitch,
        };
    }
    using Camera = omath::projection::Camera<ViewAngles, decltype(ViewMatrix<float>), decltype(PerspectiveProjectionMatrix<float>)>;


    // Camera(const Vector3& position, const ViewAnglesType& viewAngles, const ViewPort& viewPort,
    // const Angle<float, 0.f, 180.f, AngleFlags::Clamped>& fov, const float near, const float far,
    // const std::function<ViewMatFunc>& viewMatFunc, const std::function<ProjectionFunc>& projFunc)

    inline Camera CreateCamera(const Vector3& position, const auto& viewAngles, const projection::ViewPort& viewPort,
               const Angle<float, 0.f, 180.f, AngleFlags::Clamped>& fov, const float near, const float far)
    {
        return Camera(position, viewAngles, viewPort, fov, near, far, ViewMatrix<float>, PerspectiveProjectionMatrix<float>);
    }
} // namespace omath::source
