#include "omath/collision/epa_algorithm.hpp" // Epa<Collider> + GjkAlgorithmWithSimplex<Collider>
#include "omath/collision/gjk_algorithm.hpp"
#include "omath/collision/simplex.hpp"
#include "omath/engines/source_engine/collider.hpp"
#include "omath/engines/source_engine/mesh.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>
#include <memory_resource>

using Mesh = omath::source_engine::Mesh;
using Collider = omath::source_engine::MeshCollider;
using Gjk = omath::collision::GjkAlgorithm<Collider>;
using EPA = omath::collision::Epa<Collider>;

TEST(UnitTestEpa, TestCollisionTrue)
{
    // Unit cube [-1,1]^3
    std::vector<omath::primitives::Vertex<>> vbo = {
        { .position={-1.f, -1.f, -1.f}, .normal={}, .uv={} },
        { .position={-1.f, -1.f,  1.f}, .normal={}, .uv={} },
        { .position={-1.f,  1.f, -1.f}, .normal={}, .uv={} },
        { .position={-1.f,  1.f,  1.f}, .normal={}, .uv={} },
        { .position={ 1.f,  1.f,  1.f}, .normal={}, .uv={} },
        { .position={ 1.f,  1.f, -1.f}, .normal={}, .uv={} },
        { .position={ 1.f, -1.f,  1.f}, .normal={}, .uv={} },
        { .position={ 1.f, -1.f, -1.f}, .normal={}, .uv={} }
    };
    std::vector<omath::Vector3<std::uint32_t>> vao; // not needed

    Mesh a(vbo, vao, {1, 1, 1});
    Mesh b(vbo, vao, {1, 1, 1});

    // Overlap along +X by 0.5
    a.set_origin({0, 0, 0});
    b.set_origin({0.5f, 0, 0});

    Collider A(a), B(b);

    // GJK
    auto [hit, simplex] = Gjk::is_collide_with_simplex_info(A, B);
    ASSERT_TRUE(hit) << "GJK should report collision";

    // EPA
    EPA::Params params;
    auto pool = std::make_shared<std::pmr::monotonic_buffer_resource>(1024);
    params.max_iterations = 64;
    params.tolerance = 1e-4f;
    auto epa = EPA::solve(A, B, simplex, params, *pool);
    ASSERT_TRUE(epa.has_value()) << "EPA should converge";

    // Normal is unit
    EXPECT_NEAR(epa->normal.dot(epa->normal), 1.0f, 1e-5f);

    // For this setup, depth ≈ 1.5 (2 - 0.5)
    EXPECT_NEAR(epa->depth, 1.5f, 1e-3f);

    // Normal axis sanity: near X axis
    EXPECT_NEAR(std::abs(epa->normal.x), 1.0f, 1e-3f);
    EXPECT_NEAR(epa->normal.y, 0.0f, 1e-3f);
    EXPECT_NEAR(epa->normal.z, 0.0f, 1e-3f);

    // Try both signs with a tiny margin (avoid grazing contacts)
    constexpr float margin = 1.0f + 1e-3f;
    const auto pen = epa->penetration_vector;

    Mesh b_plus = b;
    b_plus.set_origin(b_plus.get_origin() + pen * margin);
    Mesh b_minus = b;
    b_minus.set_origin(b_minus.get_origin() - pen * margin);

    Collider B_plus(b_plus), B_minus(b_minus);

    const bool sep_plus = !Gjk::is_collide_with_simplex_info(A, B_plus).hit;
    const bool sep_minus = !Gjk::is_collide_with_simplex_info(A, B_minus).hit;

    // Exactly one direction should separate
    EXPECT_NE(sep_plus, sep_minus) << "Exactly one of ±penetration must separate";

    // Optional: pick the resolving direction and assert round-trip
    const auto resolve = sep_plus ? (pen * margin) : (-pen * margin);

    Mesh b_resolved = b;
    b_resolved.set_origin(b_resolved.get_origin() + resolve);
    EXPECT_FALSE(Gjk::is_collide(A, Collider(b_resolved))) << "Resolved position should be non-colliding";

    // Moving the other way should still collide
    Mesh b_wrong = b;
    b_wrong.set_origin(b_wrong.get_origin() - resolve);
    EXPECT_TRUE(Gjk::is_collide(A, Collider(b_wrong)));
}
TEST(UnitTestEpa, TestCollisionTrue2)
{
    std::vector<omath::primitives::Vertex<>> vbo = {
        { .position={ -1.f, -1.f, -1.f }, .normal={}, .uv={} },
        { .position={ -1.f, -1.f,  1.f }, .normal={}, .uv={} },
        { .position={ -1.f,  1.f, -1.f }, .normal={}, .uv={} },
        { .position={ -1.f,  1.f,  1.f }, .normal={}, .uv={} },
        { .position={  1.f,  1.f,  1.f }, .normal={}, .uv={} },
        { .position={  1.f,  1.f, -1.f }, .normal={}, .uv={} },
        { .position={  1.f, -1.f,  1.f }, .normal={}, .uv={} },
        { .position={  1.f, -1.f, -1.f }, .normal={}, .uv={} }
    };
    std::vector<omath::Vector3<std::uint32_t>> vao; // not needed

    Mesh a(vbo, vao, {1, 1, 1});
    Mesh b(vbo, vao, {1, 1, 1});

    // Overlap along +X by 0.5
    a.set_origin({0, 0, 0});
    b.set_origin({0.5f, 0, 0});

    Collider A(a), B(b);

    // --- GJK must detect collision and provide simplex ---
    auto gjk = Gjk::is_collide_with_simplex_info(A, B);
    ASSERT_TRUE(gjk.hit) << "GJK should report collision for overlapping cubes";
    // --- EPA penetration ---
    EPA::Params params;
    params.max_iterations = 64;
    params.tolerance = 1e-4f;
    auto pool = std::make_shared<std::pmr::monotonic_buffer_resource>(1024);
    auto epa = EPA::solve(A, B, gjk.simplex, params, *pool);
    ASSERT_TRUE(epa.has_value()) << "EPA should converge";

    // Normal is unit-length
    EXPECT_NEAR(epa->normal.dot(epa->normal), 1.0f, 1e-5f);

    // For centers at 0 and +0.5 and half-extent 1 -> depth ≈ 1.5
    EXPECT_NEAR(epa->depth, 1.5f, 1e-3f);

    // Axis sanity: mostly X
    EXPECT_NEAR(std::abs(epa->normal.x), 1.0f, 1e-3f);
    EXPECT_NEAR(epa->normal.y, 0.0f, 1e-3f);
    EXPECT_NEAR(epa->normal.z, 0.0f, 1e-3f);

    constexpr float margin = 1.0f + 1e-3f; // tiny slack to avoid grazing
    const auto pen = epa->normal * epa->depth;

    // Apply once: B + pen must separate; the opposite must still collide
    Mesh b_resolved = b;
    b_resolved.set_origin(b_resolved.get_origin() + pen * margin);
    EXPECT_FALSE(Gjk::is_collide(A, Collider(b_resolved))) << "Applying penetration should separate";

    Mesh b_wrong = b;
    b_wrong.set_origin(b_wrong.get_origin() - pen * margin);
    EXPECT_TRUE(Gjk::is_collide(A, Collider(b_wrong))) << "Opposite direction should still intersect";

    // Some book-keeping sanity
    EXPECT_GT(epa->iterations, 0);
    EXPECT_LT(epa->iterations, params.max_iterations);
    EXPECT_GE(epa->num_faces, 4);
    EXPECT_GT(epa->num_vertices, 4);
}
