//
// Created by Orange on 6/3/2026.
//

#pragma once
#include "constants.hpp"
#include "omath/3d_primitives/mesh.hpp"
#include "traits/mesh_trait.hpp"

namespace omath::rage_engine
{
    using Mesh = primitives::Mesh<Mat4X4, ViewAngles, MeshTrait>;
} // namespace omath::rage_engine
