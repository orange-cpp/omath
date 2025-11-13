//
// Created by Vlad on 11/13/2025.
//
#include "omath/collision/gjk_algorithm.hpp"
#include "omath/engines/source_engine/collider.hpp"
#include <gtest/gtest.h>
#include <omath/collision/epa_algorithm.hpp>
#include <omath/engines/source_engine/mesh.hpp>

namespace
{
    const omath::source_engine::Mesh mesh = {{{-1.f, -1.f, -1.f},
                                              {-1.f, -1.f, 1.f},
                                              {-1.f, 1.f, -1.f},
                                              {-1.f, 1.f, 1.f},
                                              {1.f, 1.f, 1.f},
                                              {1.f, 1.f, -1.f},
                                              {1.f, -1.f, 1.f},
                                              {1.f, -1.f, -1.f}},
                                             {}};
}
TEST(UnitTestEpa, TestCollisionTrue)
{
    std::vector<omath::Vector3<float>> vbo = {{-1, -1, -1}, {-1, -1, 1}, {-1, 1, -1}, {-1, 1, 1},
                                              {1, 1, 1},    {1, 1, -1},  {1, -1, 1},  {1, -1, -1}};
    std::vector<omath::Vector3<std::size_t>> vao; // not needed for GJK/EPA

    omath::source_engine::Mesh a(vbo, vao, {1, 1, 1});
    omath::source_engine::Mesh b(vbo, vao, {1, 1, 1});

    a.set_origin({0, 0, 0});
    b.set_origin({0.5f, 0, 0}); // slight overlap

    const omath::source_engine::MeshCollider collider_a(mesh);

    omath::source_engine::MeshCollider A(a), B(b);

    // 1) GJK → final simplex
    using Gjk = omath::collision::GjkAlgorithm<omath::source_engine::MeshCollider>;

    auto gjk = Gjk::is_collide_with_simplex_info(A, B);
    if (!gjk.hit)
    {
        std::cout << "No collision\n";
    }
    using Epa = omath::collision::Epa<omath::source_engine::MeshCollider>;
    // 2) EPA → normal/depth
    Epa::Params params;
    params.max_iterations = 64;
    params.tolerance = 1e-4f;
    auto epa = Epa::solve(A, B, gjk.simplex, params);

    if (!epa.success)
    {
        std::cout << "EPA failed\n";
    }
}