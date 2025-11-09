//
// Created by Vladislav on 09.11.2025.
//
#pragma once
#include <omath/engines/source_engine/constants.hpp>
#include <omath/engines/source_engine/formulas.hpp>

namespace omath::source_engine
{
    class MeshTrait final
    {
    public:
        [[nodiscard]]
        static Mat4X4 rotation_matrix(const ViewAngles& rotation)
        {
            return source_engine::rotation_matrix(rotation);
        }
    };
} // namespace omath::source_engine