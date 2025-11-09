//
// Created by Vladislav on 09.11.2025.
//
#pragma once
#include <omath/engines/opengl_engine/constants.hpp>
#include <omath/engines/opengl_engine/formulas.hpp>

namespace omath::opengl_engine
{
    class MeshTrait final
    {
    public:
        [[nodiscard]]
        static Mat4X4 rotation_matrix(const ViewAngles& rotation)
        {
            return opengl_engine::rotation_matrix(rotation);
        }
    };
} // namespace omath::opengl_engine