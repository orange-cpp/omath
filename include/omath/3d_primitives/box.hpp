//
// Created by Vlad on 4/18/2025.
//

#pragma once
#include "mesh.hpp"
#include "omath/engines/opengl_engine/camera.hpp"
#include "omath/engines/opengl_engine/traits/mesh_trait.hpp"
#include "omath/linear_algebra/triangle.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <array>

namespace omath::primitives
{
    using BoxMesh = Mesh<opengl_engine::Mat4X4, opengl_engine::ViewAngles, opengl_engine::MeshTrait, Vertex<>,
                         std::array<Vector3<float>, 8>, std::array<Vector3<std::uint32_t>, 12>>;

    template<class BoxMeshType = BoxMesh>
    [[nodiscard]]
    BoxMeshType
    create_box(const Vector3<float>& top, const Vector3<float>& bottom, const Vector3<float>& dir_forward,
               const Vector3<float>& dir_right, const float ratio = 4.f) noexcept
    {
        const auto height = top.distance_to(bottom);
        const auto side_size = height / ratio;

        // corner layout (0‑3 bottom, 4‑7 top)
        std::array<Vector3<float>, 8> p;
        p[0] = bottom + (dir_forward + dir_right) * side_size; // front‑right‑bottom
        p[1] = bottom + (dir_forward - dir_right) * side_size; // front‑left‑bottom
        p[2] = bottom + (-dir_forward + dir_right) * side_size; // back‑right‑bottom
        p[3] = bottom + (-dir_forward - dir_right) * side_size; // back‑left‑bottom
        p[4] = top + (dir_forward + dir_right) * side_size; // front‑right‑top
        p[5] = top + (dir_forward - dir_right) * side_size; // front‑left‑top
        p[6] = top + (-dir_forward + dir_right) * side_size; // back‑right‑top
        p[7] = top + (-dir_forward - dir_right) * side_size; // back‑left‑top

        std::array<Vector3<std::uint32_t>, 12> poly;

        // bottom face (+Y up ⇒ wind CW when viewed from above)
        poly[0] = {0, 2, 3};
        poly[1] = {0, 3, 1};

        // top face
        poly[2] = {4, 7, 6};
        poly[3] = {4, 5, 7};

        // front face
        poly[4] = {0, 5, 1};
        poly[5] = {0, 4, 5};

        // right face
        poly[6] = {0, 6, 2};
        poly[7] = {0, 4, 6};

        // back face
        poly[8] = {2, 7, 3};
        poly[9] = {2, 6, 7};

        // left face
        poly[10] = {1, 7, 5};
        poly[11] = {1, 3, 7};

        return BoxMeshType{std::move(p), std::move(poly)};
    }
} // namespace omath::primitives
