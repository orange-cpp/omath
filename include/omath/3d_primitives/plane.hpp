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
    using PlaneMesh = Mesh<opengl_engine::Mat4X4, opengl_engine::ViewAngles, opengl_engine::MeshTrait, Vertex<>,
                           std::array<Vector3<float>, 4>, std::array<Vector3<std::uint32_t>, 6>>;
    [[nodiscard]]
    PlaneMesh create_plane(const Vector3<float>& vertex_a, const Vector3<float>& vertex_b,
                           const Vector3<float>& direction, const float size) noexcept
    {
        const auto second_vertex_a = vertex_a + direction * size;
        const auto second_vertex_b = vertex_b + direction * size;

        std::array<Vector3<float>, 4> grid = {vertex_a, vertex_b, second_vertex_a, second_vertex_b};

        std::array<Vector3<std::uint32_t>, 6> poly;
        poly[0] = {};

        return PlaneMesh(std::move(grid), std::move(poly));
    }
} // namespace omath::primitives
