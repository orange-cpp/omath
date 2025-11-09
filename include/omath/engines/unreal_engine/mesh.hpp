//
// Created by Vladislav on 09.11.2025.
//
#pragma once
#include "constants.hpp"
#include "omath/3d_primitives/mesh.hpp"
#include "traits/mesh_trait.hpp"

namespace omath::unreal_engine
{
    using Mesh = primitives::Mesh<Mat4X4, ViewAngles, MeshTrait, float>;
}