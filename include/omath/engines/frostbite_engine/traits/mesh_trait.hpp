//
// Created by Vladislav on 09.11.2025.
//
#pragma once
#include <omath/engines/frostbite_engine/constants.hpp>
#include <omath/engines/frostbite_engine/formulas.hpp>

namespace omath::frostbite_engine
{
    class MeshTrait final
    {
    public:
        [[nodiscard]]
        static Mat4X4 rotation_matrix(const ViewAngles& rotation)
        {
            return frostbite_engine::rotation_matrix(rotation);
        }
    };
} // namespace omath::frostbite_engine