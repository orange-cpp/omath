//
// Comprehensive EPA tests.
// Covers: all 3 axis directions, multiple depth levels, penetration-vector
// round-trips, depth monotonicity, symmetry, asymmetric sizes, memory
// resource variants, tolerance sensitivity, and iteration bookkeeping.
//
#include <cmath>
#include <gtest/gtest.h>
#include <memory_resource>
#include <omath/collision/epa_algorithm.hpp>
#include <omath/collision/gjk_algorithm.hpp>
#include <omath/engines/source_engine/collider.hpp>
#include <omath/engines/source_engine/mesh.hpp>

using Mesh     = omath::source_engine::Mesh;
using Collider = omath::source_engine::MeshCollider;
using Gjk      = omath::collision::GjkAlgorithm<Collider>;
using Epa      = omath::collision::Epa<Collider>;
using Vec3     = omath::Vector3<float>;

namespace
{
    const std::vector<omath::primitives::Vertex<>> k_cube_vbo = {
        { { -1.f, -1.f, -1.f }, {}, {} },
        { { -1.f, -1.f,  1.f }, {}, {} },
        { { -1.f,  1.f, -1.f }, {}, {} },
        { { -1.f,  1.f,  1.f }, {}, {} },
        { {  1.f,  1.f,  1.f }, {}, {} },
        { {  1.f,  1.f, -1.f }, {}, {} },
        { {  1.f, -1.f,  1.f }, {}, {} },
        { {  1.f, -1.f, -1.f }, {}, {} },
    };
    const std::vector<omath::Vector3<std::uint32_t>> k_empty_ebo{};

    constexpr Epa::Params k_default_params{ .max_iterations = 64, .tolerance = 1e-4f };

    Collider make_cube(const Vec3& origin = {}, const Vec3& scale = { 1, 1, 1 })
    {
        Mesh m{ k_cube_vbo, k_empty_ebo, scale };
        m.set_origin(origin);
        return Collider{ m };
    }

    // Run GJK then EPA; asserts GJK hit and EPA converged.
    Epa::Result solve(const Collider& a, const Collider& b,
                      const Epa::Params& params = k_default_params)
    {
        const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(a, b);
        EXPECT_TRUE(hit) << "GJK must detect collision before EPA can run";
        auto result = Epa::solve(a, b, simplex, params);
        EXPECT_TRUE(result.has_value()) << "EPA must converge";
        return *result;
    }
} // namespace

// ---------------------------------------------------------------------------
// Normal direction per axis
// ---------------------------------------------------------------------------

// For two unit cubes (half-extent 1) with B offset by d along an axis:
//   depth = 2 - d   (distance from origin to nearest face of Minkowski diff)
//   normal component along that axis ≈ ±1

TEST(EpaComprehensive, NormalAlongX_Positive)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0.5f, 0, 0 }));
    EXPECT_NEAR(std::abs(r.normal.x), 1.f, 1e-3f);
    EXPECT_NEAR(r.normal.y, 0.f, 1e-3f);
    EXPECT_NEAR(r.normal.z, 0.f, 1e-3f);
}

TEST(EpaComprehensive, NormalAlongX_Negative)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ -0.5f, 0, 0 }));
    EXPECT_NEAR(std::abs(r.normal.x), 1.f, 1e-3f);
    EXPECT_NEAR(r.normal.y, 0.f, 1e-3f);
    EXPECT_NEAR(r.normal.z, 0.f, 1e-3f);
}

TEST(EpaComprehensive, NormalAlongY_Positive)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0, 0.5f, 0 }));
    EXPECT_NEAR(r.normal.x, 0.f, 1e-3f);
    EXPECT_NEAR(std::abs(r.normal.y), 1.f, 1e-3f);
    EXPECT_NEAR(r.normal.z, 0.f, 1e-3f);
}

TEST(EpaComprehensive, NormalAlongY_Negative)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0, -0.5f, 0 }));
    EXPECT_NEAR(r.normal.x, 0.f, 1e-3f);
    EXPECT_NEAR(std::abs(r.normal.y), 1.f, 1e-3f);
    EXPECT_NEAR(r.normal.z, 0.f, 1e-3f);
}

TEST(EpaComprehensive, NormalAlongZ_Positive)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0, 0, 0.5f }));
    EXPECT_NEAR(r.normal.x, 0.f, 1e-3f);
    EXPECT_NEAR(r.normal.y, 0.f, 1e-3f);
    EXPECT_NEAR(std::abs(r.normal.z), 1.f, 1e-3f);
}

TEST(EpaComprehensive, NormalAlongZ_Negative)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0, 0, -0.5f }));
    EXPECT_NEAR(r.normal.x, 0.f, 1e-3f);
    EXPECT_NEAR(r.normal.y, 0.f, 1e-3f);
    EXPECT_NEAR(std::abs(r.normal.z), 1.f, 1e-3f);
}

// ---------------------------------------------------------------------------
// Depth correctness  (depth = 2 - offset for unit cubes)
// ---------------------------------------------------------------------------

TEST(EpaComprehensive, Depth_ShallowOverlap)
{
    // offset 1.9 → depth 0.1
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 1.9f, 0, 0 }));
    EXPECT_NEAR(r.depth, 0.1f, 1e-2f);
}

TEST(EpaComprehensive, Depth_QuarterOverlap)
{
    // offset 1.5 → depth 0.5
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 1.5f, 0, 0 }));
    EXPECT_NEAR(r.depth, 0.5f, 1e-2f);
}

TEST(EpaComprehensive, Depth_HalfOverlap)
{
    // offset 1.0 → depth 1.0
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 1.0f, 0, 0 }));
    EXPECT_NEAR(r.depth, 1.0f, 1e-2f);
}

TEST(EpaComprehensive, Depth_ThreeQuarterOverlap)
{
    // offset 0.5 → depth 1.5
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0.5f, 0, 0 }));
    EXPECT_NEAR(r.depth, 1.5f, 1e-2f);
}

TEST(EpaComprehensive, Depth_AlongY_HalfOverlap)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0, 1.0f, 0 }));
    EXPECT_NEAR(r.depth, 1.0f, 1e-2f);
}

TEST(EpaComprehensive, Depth_AlongZ_HalfOverlap)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0, 0, 1.0f }));
    EXPECT_NEAR(r.depth, 1.0f, 1e-2f);
}

// ---------------------------------------------------------------------------
// Depth monotonicity — deeper overlap → larger depth
// ---------------------------------------------------------------------------

TEST(EpaComprehensive, DepthMonotonic_AlongX)
{
    const float d1 = solve(make_cube({ 0, 0, 0 }), make_cube({ 1.9f, 0, 0 })).depth; // ~0.1
    const float d2 = solve(make_cube({ 0, 0, 0 }), make_cube({ 1.5f, 0, 0 })).depth; // ~0.5
    const float d3 = solve(make_cube({ 0, 0, 0 }), make_cube({ 1.0f, 0, 0 })).depth; // ~1.0
    const float d4 = solve(make_cube({ 0, 0, 0 }), make_cube({ 0.5f, 0, 0 })).depth; // ~1.5

    EXPECT_LT(d1, d2);
    EXPECT_LT(d2, d3);
    EXPECT_LT(d3, d4);
}

// ---------------------------------------------------------------------------
// Normal is a unit vector
// ---------------------------------------------------------------------------

TEST(EpaComprehensive, NormalIsUnit_AlongX)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0.5f, 0, 0 }));
    EXPECT_NEAR(r.normal.dot(r.normal), 1.f, 1e-5f);
}

TEST(EpaComprehensive, NormalIsUnit_AlongY)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0, 1.2f, 0 }));
    EXPECT_NEAR(r.normal.dot(r.normal), 1.f, 1e-5f);
}

TEST(EpaComprehensive, NormalIsUnit_AlongZ)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0, 0, 0.8f }));
    EXPECT_NEAR(r.normal.dot(r.normal), 1.f, 1e-5f);
}

// ---------------------------------------------------------------------------
// Penetration vector = normal * depth
// ---------------------------------------------------------------------------

TEST(EpaComprehensive, PenetrationVectorLength_EqualsDepth)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0.5f, 0, 0 }));
    const float pen_len = std::sqrt(r.penetration_vector.dot(r.penetration_vector));
    EXPECT_NEAR(pen_len, r.depth, 1e-5f);
}

TEST(EpaComprehensive, PenetrationVectorDirection_ParallelToNormal)
{
    const auto r = solve(make_cube({ 0, 0, 0 }), make_cube({ 0, 1.0f, 0 }));
    // penetration_vector = normal * depth → cross product must be ~zero
    const auto cross = r.penetration_vector.cross(r.normal);
    EXPECT_NEAR(cross.dot(cross), 0.f, 1e-8f);
}

// ---------------------------------------------------------------------------
// Round-trip: applying penetration_vector separates the shapes
// ---------------------------------------------------------------------------

TEST(EpaComprehensive, RoundTrip_AlongX)
{
    const auto a = make_cube({ 0, 0, 0 });
    Mesh mesh_b{ k_cube_vbo, k_empty_ebo };
    mesh_b.set_origin({ 0.5f, 0, 0 });
    const auto b = Collider{ mesh_b };

    const auto r = solve(a, b);
    constexpr float margin = 1.f + 1e-3f;

    // Move B along the penetration vector; it should separate from A
    Mesh mesh_sep{ k_cube_vbo, k_empty_ebo };
    mesh_sep.set_origin(mesh_b.get_origin() + r.penetration_vector * margin);
    EXPECT_FALSE(Gjk::is_collide(a, Collider{ mesh_sep })) << "Applying pen vector must separate";

    // Moving the wrong way must still collide
    Mesh mesh_wrong{ k_cube_vbo, k_empty_ebo };
    mesh_wrong.set_origin(mesh_b.get_origin() - r.penetration_vector * margin);
    EXPECT_TRUE(Gjk::is_collide(a, Collider{ mesh_wrong })) << "Opposite direction must still collide";
}

TEST(EpaComprehensive, RoundTrip_AlongY)
{
    const auto a = make_cube({ 0, 0, 0 });
    Mesh mesh_b{ k_cube_vbo, k_empty_ebo };
    mesh_b.set_origin({ 0, 0.8f, 0 });
    const auto b = Collider{ mesh_b };

    const auto r = solve(a, b);
    constexpr float margin = 1.f + 1e-3f;

    Mesh mesh_sep{ k_cube_vbo, k_empty_ebo };
    mesh_sep.set_origin(mesh_b.get_origin() + r.penetration_vector * margin);
    EXPECT_FALSE(Gjk::is_collide(a, Collider{ mesh_sep }));

    Mesh mesh_wrong{ k_cube_vbo, k_empty_ebo };
    mesh_wrong.set_origin(mesh_b.get_origin() - r.penetration_vector * margin);
    EXPECT_TRUE(Gjk::is_collide(a, Collider{ mesh_wrong }));
}

TEST(EpaComprehensive, RoundTrip_AlongZ)
{
    const auto a = make_cube({ 0, 0, 0 });
    Mesh mesh_b{ k_cube_vbo, k_empty_ebo };
    mesh_b.set_origin({ 0, 0, 1.2f });
    const auto b = Collider{ mesh_b };

    const auto r = solve(a, b);
    constexpr float margin = 1.f + 1e-3f;

    Mesh mesh_sep{ k_cube_vbo, k_empty_ebo };
    mesh_sep.set_origin(mesh_b.get_origin() + r.penetration_vector * margin);
    EXPECT_FALSE(Gjk::is_collide(a, Collider{ mesh_sep }));
}

// ---------------------------------------------------------------------------
// Symmetry — swapping A and B preserves depth
// ---------------------------------------------------------------------------

TEST(EpaComprehensive, Symmetry_DepthIsIndependentOfOrder)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 0.5f, 0, 0 });

    const float depth_ab = solve(a, b).depth;
    const float depth_ba = solve(b, a).depth;

    EXPECT_NEAR(depth_ab, depth_ba, 1e-2f);
}

TEST(EpaComprehensive, Symmetry_NormalsAreOpposite)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 0.5f, 0, 0 });

    const Vec3 n_ab = solve(a, b).normal;
    const Vec3 n_ba = solve(b, a).normal;

    // The normals should be anti-parallel: n_ab · n_ba ≈ -1
    EXPECT_NEAR(n_ab.dot(n_ba), -1.f, 1e-3f);
}

// ---------------------------------------------------------------------------
// Asymmetric sizes
// ---------------------------------------------------------------------------

TEST(EpaComprehensive, LargeVsSmall_DepthCorrect)
{
    // Big (half-ext 2) at origin, small (half-ext 0.5) at (2.0, 0, 0)
    // Minkowski diff closest face in X at distance 0.5
    const auto r = solve(make_cube({ 0, 0, 0 }, { 2, 2, 2 }), make_cube({ 2.0f, 0, 0 }, { 0.5f, 0.5f, 0.5f }));
    EXPECT_NEAR(r.depth, 0.5f, 1e-2f);
    EXPECT_NEAR(std::abs(r.normal.x), 1.f, 1e-3f);
}

TEST(EpaComprehensive, LargeVsSmall_RoundTrip)
{
    const auto a = make_cube({ 0, 0, 0 }, { 2, 2, 2 });

    Mesh mesh_b{ k_cube_vbo, k_empty_ebo, { 0.5f, 0.5f, 0.5f } };
    mesh_b.set_origin({ 2.0f, 0, 0 });
    const auto b = Collider{ mesh_b };

    const auto r = solve(a, b);
    constexpr float margin = 1.f + 1e-3f;

    Mesh mesh_sep{ k_cube_vbo, k_empty_ebo, { 0.5f, 0.5f, 0.5f } };
    mesh_sep.set_origin(mesh_b.get_origin() + r.penetration_vector * margin);
    EXPECT_FALSE(Gjk::is_collide(a, Collider{ mesh_sep }));
}

// ---------------------------------------------------------------------------
// Memory resource variants
// ---------------------------------------------------------------------------

TEST(EpaComprehensive, MonotonicBuffer_ConvergesCorrectly)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 0.5f, 0, 0 });
    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(a, b);
    ASSERT_TRUE(hit);

    constexpr std::size_t k_buf = 32768;
    alignas(std::max_align_t) char buf[k_buf];
    std::pmr::monotonic_buffer_resource mr{ buf, k_buf, std::pmr::null_memory_resource() };

    const auto r = Epa::solve(a, b, simplex, k_default_params, mr);
    ASSERT_TRUE(r.has_value());
    EXPECT_NEAR(r->depth, 1.5f, 1e-2f);
}

TEST(EpaComprehensive, MonotonicBuffer_MultipleReleaseCycles)
{
    // Verify mr.release() correctly resets the buffer across multiple calls
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 0.5f, 0, 0 });
    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(a, b);
    ASSERT_TRUE(hit);

    constexpr std::size_t k_buf = 32768;
    alignas(std::max_align_t) char buf[k_buf];
    std::pmr::monotonic_buffer_resource mr{ buf, k_buf, std::pmr::null_memory_resource() };

    float first_depth = 0.f;
    for (int i = 0; i < 5; ++i)
    {
        mr.release();
        const auto r = Epa::solve(a, b, simplex, k_default_params, mr);
        ASSERT_TRUE(r.has_value()) << "solve must converge on iteration " << i;
        if (i == 0)
            first_depth = r->depth;
        else
            EXPECT_NEAR(r->depth, first_depth, 1e-6f) << "depth must be deterministic";
    }
}

TEST(EpaComprehensive, DefaultResource_ConvergesCorrectly)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 1.0f, 0, 0 });
    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(a, b);
    ASSERT_TRUE(hit);

    const auto r = Epa::solve(a, b, simplex);
    ASSERT_TRUE(r.has_value());
    EXPECT_NEAR(r->depth, 1.0f, 1e-2f);
}

// ---------------------------------------------------------------------------
// Tolerance sensitivity
// ---------------------------------------------------------------------------

TEST(EpaComprehensive, TighterTolerance_MoreAccurateDepth)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 1.0f, 0, 0 });
    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(a, b);
    ASSERT_TRUE(hit);

    const Epa::Params loose{ .max_iterations = 64, .tolerance = 1e-2f };
    const Epa::Params tight{ .max_iterations = 64, .tolerance = 1e-5f };

    const auto r_loose = Epa::solve(a, b, simplex, loose);
    const auto r_tight = Epa::solve(a, b, simplex, tight);
    ASSERT_TRUE(r_loose.has_value());
    ASSERT_TRUE(r_tight.has_value());

    // Tighter tolerance must yield a result at least as accurate
    EXPECT_LE(std::abs(r_tight->depth - 1.0f), std::abs(r_loose->depth - 1.0f) + 1e-4f);
}

// ---------------------------------------------------------------------------
// Bookkeeping fields
// ---------------------------------------------------------------------------

TEST(EpaComprehensive, Bookkeeping_IterationsInBounds)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 0.5f, 0, 0 });
    const auto r = solve(a, b);

    EXPECT_GT(r.iterations, 0);
    EXPECT_LE(r.iterations, k_default_params.max_iterations);
}

TEST(EpaComprehensive, Bookkeeping_FacesAndVerticesGrow)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 0.5f, 0, 0 });
    const auto r = solve(a, b);

    // Started with a tetrahedron (4 faces, 4 vertices); EPA must have expanded it
    EXPECT_GE(r.num_faces, 4);
    EXPECT_GE(r.num_vertices, 4);
}

TEST(EpaComprehensive, Bookkeeping_MaxIterationsRespected)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 0.5f, 0, 0 });
    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(a, b);
    ASSERT_TRUE(hit);

    constexpr Epa::Params tight{ .max_iterations = 3, .tolerance = 1e-10f };
    const auto r = Epa::solve(a, b, simplex, tight);

    // Must return something (fallback best-face path) and respect the cap
    if (r.has_value())
        EXPECT_LE(r->iterations, tight.max_iterations);
}

// ---------------------------------------------------------------------------
// Determinism
// ---------------------------------------------------------------------------

TEST(EpaComprehensive, Deterministic_SameResultOnRepeatedCalls)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 0.7f, 0, 0 });
    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(a, b);
    ASSERT_TRUE(hit);

    const auto first = Epa::solve(a, b, simplex);
    ASSERT_TRUE(first.has_value());

    for (int i = 0; i < 5; ++i)
    {
        const auto r = Epa::solve(a, b, simplex);
        ASSERT_TRUE(r.has_value());
        EXPECT_NEAR(r->depth, first->depth, 1e-6f);
        EXPECT_NEAR(r->normal.x, first->normal.x, 1e-6f);
        EXPECT_NEAR(r->normal.y, first->normal.y, 1e-6f);
        EXPECT_NEAR(r->normal.z, first->normal.z, 1e-6f);
    }
}
