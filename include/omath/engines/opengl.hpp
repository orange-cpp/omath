//
// Created by Orange on 11/23/2024.
//
#pragma once
#include "omath/Vector3.hpp"
#include "omath/Mat.hpp"


namespace omath::opengl
{
    constexpr Vector3 kAbsUp = {0, 1, 0};
    constexpr Vector3 kAbsRight = {1, 0, 0};
    constexpr Vector3 kAbsForward = {0, 0, -1};

    template<class Type> requires std::is_floating_point_v<Type> || std::is_integral_v<Type>
    [[nodiscard]] Mat<4, 4, Type, MatStoreType::COLUMN_MAJOR> PerspectiveProjectionMatrix()
    {

    }

    template<class Type> requires std::is_floating_point_v<Type> || std::is_integral_v<Type>
    [[nodiscard]] Mat<4, 4, Type, MatStoreType::COLUMN_MAJOR> PerspectiveProjectionMatrix(
        const float fieldOfView, const Type &aspectRatio, const Type &near, const Type &far)
    {
        const float fovHalfTan = std::tan(angles::DegreesToRadians(fieldOfView) / 2);

        return
        {
            {static_cast<Type>(1) / (aspectRatio * fovHalfTan), 0, 0, 0},
            {0, static_cast<Type>(1) / (fovHalfTan), 0, 0},
            {0, 0, -(far + near) / (far - near), -(static_cast<Type>(2) * far * near) / (far - near)},
            {0, 0, -1, 0},
        };
    }
}
