//
// Created by Vladislav on 26.01.2026.
//
#pragma once

namespace omath::trigonometry
{
    template<class RotationAxisX, class RotationAxisY, class RotationAxisZ>
    struct RotationAngles final
    {
        RotationAxisX rotation_axis_x;
        RotationAxisY rotation_axis_y;
        RotationAxisZ rotation_axis_z;
    };
}