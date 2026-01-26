//
// Created by Vlad on 8/28/2025.
//

#pragma once
#include "mesh.hpp"
#include "omath/engines/opengl_engine/camera.hpp"
#include "omath/engines/opengl_engine/mesh.hpp"
#include "omath/engines/opengl_engine/traits/mesh_trait.hpp"
#include "omath/linear_algebra/triangle.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <array>

namespace omath::primitives
{
    using PlaneMesh = Mesh<opengl_engine::Mat4X4, opengl_engine::ViewAngles, opengl_engine::MeshTrait, Vector3<float>,
                           std::array<Vertex<>, 8>, std::array<Vector3<std::uint32_t>, 12>>;
    [[nodiscard]]
    std::array<Triangle<Vector3<float>>, 2> create_plane(const Vector3<float>& vertex_a, const Vector3<float>& vertex_b,
                                                         const Vector3<float>& direction, float size) noexcept
    {
        const auto second_vertex_a = vertex_a + direction * size;
        return std::array{Triangle{second_vertex_a, vertex_a, vertex_b},
                          Triangle{second_vertex_a, vertex_b + direction * size, vertex_b}};
    }
} // namespace omath::primitives
