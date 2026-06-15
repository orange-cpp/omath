//
// Created by Orange on 6/3/2026.
//

#pragma once
#include <omath/engines/rage_engine/constants.hpp>
#include <omath/engines/rage_engine/formulas.hpp>

namespace omath::rage_engine
{
    class MeshTrait final
    {
    public:
        [[nodiscard("rotation matrix result should not be discarded")]]
        static Mat4X4 rotation_matrix(const ViewAngles& rotation)
        {
            return rage_engine::rotation_matrix(rotation);
        }
    };
} // namespace omath::rage_engine
