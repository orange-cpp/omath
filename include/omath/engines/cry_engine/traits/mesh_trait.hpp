//
// Created by Vladislav on 09.11.2025.
//
#pragma once
#include <omath/engines/cry_engine/constants.hpp>
#include <omath/engines/cry_engine/formulas.hpp>

namespace omath::cry_engine
{
    class MeshTrait final
    {
    public:
        [[nodiscard]]
        static Mat4X4 rotation_matrix(const ViewAngles& rotation)
        {
            return cry_engine::rotation_matrix(rotation);
        }
    };
} // namespace omath::cry_engine