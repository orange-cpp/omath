//
// Created by Vladislav on 27.01.2026.
//

#pragma once
#include "mesh.hpp"
#include "omath/engines/source_engine/traits/mesh_trait.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <array>
namespace omath::source_engine
{
    using BoxMesh = primitives::Mesh<Mat4X4, ViewAngles, MeshTrait, primitives::Vertex<>, std::array<Vector3<float>, 8>,
                                     std::array<Vector3<std::uint32_t>, 12>>;

    using PlaneMesh = primitives::Mesh<Mat4X4, ViewAngles, MeshTrait, primitives::Vertex<>,
                           std::array<Vector3<float>, 4>, std::array<Vector3<std::uint32_t>, 2>>;
} // namespace omath::source_engine