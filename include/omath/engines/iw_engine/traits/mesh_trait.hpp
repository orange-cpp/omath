//
// Created by Vladislav on 09.11.2025.
//
#pragma once
#include <omath/engines/iw_engine/constants.hpp>
#include <omath/engines/iw_engine/formulas.hpp>

namespace omath::iw_engine
{
    class MeshTrait final
    {
    public:
        [[nodiscard("rotation matrix result should not be discarded")]]
        static Mat4X4 rotation_matrix(const ViewAngles& rotation)
        {
            return iw_engine::rotation_matrix(rotation);
        }
    };
} // namespace omath::iw_engine