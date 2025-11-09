//
// Created by Vladislav on 09.11.2025.
//

#pragma once
#include "mesh.hpp"
#include "omath/3d_primitives/mesh.hpp"
#include "omath/collision/mesh_collider.hpp"

namespace omath::source_engine
{
    using MeshCollider = collision::MeshCollider<Mesh>;
}