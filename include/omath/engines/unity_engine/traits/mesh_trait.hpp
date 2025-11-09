//
// Created by Vladislav on 09.11.2025.
//
#pragma once
#include <omath/engines/unity_engine/constants.hpp>
#include <omath/engines/unity_engine/formulas.hpp>

namespace omath::unity_engine
{
    class MeshTrait final
    {
    public:
        [[nodiscard]]
        static Mat4X4 rotation_matrix(const ViewAngles& rotation)
        {
            return unity_engine::rotation_matrix(rotation);
        }
    };
} // namespace omath::unity_engine