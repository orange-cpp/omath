//
// Created by Vladislav on 09.11.2025.
//
#pragma once
#include <omath/engines/unreal_engine/constants.hpp>
#include <omath/engines/unreal_engine/formulas.hpp>

namespace omath::unreal_engine
{
    class MeshTrait final
    {
    public:
        [[nodiscard]]
        static Mat4X4 rotation_matrix(const ViewAngles& rotation)
        {
            return unreal_engine::rotation_matrix(rotation);
        }
    };
} // namespace omath::unreal_engine