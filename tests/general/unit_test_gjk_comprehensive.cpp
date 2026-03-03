//
// Comprehensive GJK tests.
// Covers: all 6 axis directions, diagonal cases, boundary touching,
// asymmetric sizes, nesting, symmetry, simplex info, far separation.
//
#include <gtest/gtest.h>
#include <omath/collision/gjk_algorithm.hpp>
#include <omath/engines/source_engine/collider.hpp>
#include <omath/engines/source_engine/mesh.hpp>

using Mesh     = omath::source_engine::Mesh;
using Collider = omath::source_engine::MeshCollider;
using Gjk      = omath::collision::GjkAlgorithm<Collider>;
using Vec3     = omath::Vector3<float>;

namespace
{
    // Unit cube [-1, 1]^3 in local space.
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

    Collider make_cube(const Vec3& origin = {}, const Vec3& scale = { 1, 1, 1 })
    {
        Mesh m{ k_cube_vbo, k_empty_ebo, scale };
        m.set_origin(origin);
        return Collider{ m };
    }
} // namespace

// ---------------------------------------------------------------------------
// Separation — expect false
// ---------------------------------------------------------------------------

TEST(GjkComprehensive, Separated_AlongPosX)
{
    // A extends to x=1, B starts at x=1.1 → clear gap
    EXPECT_FALSE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 2.1f, 0, 0 })));
}

TEST(GjkComprehensive, Separated_AlongNegX)
{
    // B to the left of A
    EXPECT_FALSE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ -2.1f, 0, 0 })));
}

TEST(GjkComprehensive, Separated_AlongPosY)
{
    EXPECT_FALSE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 0, 2.1f, 0 })));
}

TEST(GjkComprehensive, Separated_AlongNegY)
{
    EXPECT_FALSE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 0, -2.1f, 0 })));
}

TEST(GjkComprehensive, Separated_AlongPosZ)
{
    EXPECT_FALSE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 0, 0, 2.1f })));
}

TEST(GjkComprehensive, Separated_AlongNegZ)
{
    EXPECT_FALSE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 0, 0, -2.1f })));
}

TEST(GjkComprehensive, Separated_AlongDiagonal)
{
    // All components exceed 2.0 — no overlap on any axis
    EXPECT_FALSE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 2.1f, 2.1f, 2.1f })));
}

TEST(GjkComprehensive, Separated_LargeDistance)
{
    EXPECT_FALSE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 100.f, 0, 0 })));
}

TEST(GjkComprehensive, Separated_AsymmetricSizes)
{
    // Big (scale 2, half-ext 2), small (scale 0.5, half-ext 0.5) at 2.6 → gap of 0.1
    EXPECT_FALSE(Gjk::is_collide(make_cube({ 0, 0, 0 }, { 2, 2, 2 }), make_cube({ 2.6f, 0, 0 }, { 0.5f, 0.5f, 0.5f })));
}

// ---------------------------------------------------------------------------
// Overlap — expect true
// ---------------------------------------------------------------------------

TEST(GjkComprehensive, Overlapping_AlongPosX)
{
    // B offset 1.5 → overlap depth 0.5 in X
    EXPECT_TRUE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 1.5f, 0, 0 })));
}

TEST(GjkComprehensive, Overlapping_AlongNegX)
{
    EXPECT_TRUE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ -1.5f, 0, 0 })));
}

TEST(GjkComprehensive, Overlapping_AlongPosZ)
{
    EXPECT_TRUE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 0, 0, 1.5f })));
}

TEST(GjkComprehensive, Overlapping_AlongNegZ)
{
    EXPECT_TRUE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 0, 0, -1.5f })));
}

TEST(GjkComprehensive, Overlapping_AlongDiagonalXY)
{
    // Minkowski sum extends ±2 on each axis; offset (1,1,0) is inside
    EXPECT_TRUE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 1.f, 1.f, 0.f })));
}

TEST(GjkComprehensive, Overlapping_AlongDiagonalXYZ)
{
    // All three axes overlap: (1,1,1) is inside the Minkowski sum
    EXPECT_TRUE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 1.f, 1.f, 1.f })));
}

TEST(GjkComprehensive, FullyNested_SmallInsideBig)
{
    // Small cube (half-ext 0.5) fully inside big cube (half-ext 2)
    EXPECT_TRUE(Gjk::is_collide(make_cube({ 0, 0, 0 }, { 2, 2, 2 }), make_cube({ 0, 0, 0 }, { 0.5f, 0.5f, 0.5f })));
}

TEST(GjkComprehensive, FullyNested_OffCenter)
{
    // Small at (0.5, 0, 0) still fully inside big (half-ext 2)
    EXPECT_TRUE(Gjk::is_collide(make_cube({ 0, 0, 0 }, { 2, 2, 2 }), make_cube({ 0.5f, 0, 0 }, { 0.5f, 0.5f, 0.5f })));
}

TEST(GjkComprehensive, Overlapping_AsymmetricSizes)
{
    // Big (scale 2, half-ext 2) and small (scale 0.5, half-ext 0.5) at 2.0 → overlap 0.5 in X
    EXPECT_TRUE(Gjk::is_collide(make_cube({ 0, 0, 0 }, { 2, 2, 2 }), make_cube({ 2.0f, 0, 0 }, { 0.5f, 0.5f, 0.5f })));
}

// ---------------------------------------------------------------------------
// Boundary cases
// ---------------------------------------------------------------------------

TEST(GjkComprehensive, BoundaryCase_JustColliding)
{
    // B at 1.999 — 0.001 overlap in X
    EXPECT_TRUE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 1.999f, 0, 0 })));
}

TEST(GjkComprehensive, BoundaryCase_JustSeparated)
{
    // B at 2.001 — 0.001 gap in X
    EXPECT_FALSE(Gjk::is_collide(make_cube({ 0, 0, 0 }), make_cube({ 2.001f, 0, 0 })));
}

// ---------------------------------------------------------------------------
// Symmetry
// ---------------------------------------------------------------------------

TEST(GjkComprehensive, Symmetry_WhenColliding)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 1.5f, 0, 0 });
    EXPECT_EQ(Gjk::is_collide(a, b), Gjk::is_collide(b, a));
}

TEST(GjkComprehensive, Symmetry_WhenSeparated)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 2.1f, 0.5f, 0 });
    EXPECT_EQ(Gjk::is_collide(a, b), Gjk::is_collide(b, a));
}

TEST(GjkComprehensive, Symmetry_DiagonalSeparation)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 1.5f, 1.5f, 1.5f });
    EXPECT_EQ(Gjk::is_collide(a, b), Gjk::is_collide(b, a));
}

// ---------------------------------------------------------------------------
// Simplex info
// ---------------------------------------------------------------------------

TEST(GjkComprehensive, SimplexInfo_HitProducesSimplex4)
{
    // On collision the simplex must be a full tetrahedron (4 points)
    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(make_cube({ 0, 0, 0 }), make_cube({ 0.5f, 0, 0 }));
    EXPECT_TRUE(hit);
    EXPECT_EQ(simplex.size(), 4u);
}

TEST(GjkComprehensive, SimplexInfo_MissProducesLessThan4)
{
    // On non-collision the simplex can never be a full tetrahedron
    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(make_cube({ 0, 0, 0 }), make_cube({ 2.1f, 0, 0 }));
    EXPECT_FALSE(hit);
    EXPECT_LT(simplex.size(), 4u);
}

TEST(GjkComprehensive, SimplexInfo_HitAlongY)
{
    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(make_cube({ 0, 0, 0 }), make_cube({ 0, 1.5f, 0 }));
    EXPECT_TRUE(hit);
    EXPECT_EQ(simplex.size(), 4u);
}

TEST(GjkComprehensive, SimplexInfo_HitAlongZ)
{
    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(make_cube({ 0, 0, 0 }), make_cube({ 0, 0, 1.5f }));
    EXPECT_TRUE(hit);
    EXPECT_EQ(simplex.size(), 4u);
}

TEST(GjkComprehensive, SimplexInfo_MissAlongDiagonal)
{
    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(make_cube({ 0, 0, 0 }), make_cube({ 2.1f, 2.1f, 2.1f }));
    EXPECT_FALSE(hit);
    EXPECT_LT(simplex.size(), 4u);
}

// ---------------------------------------------------------------------------
// Non-trivial geometry — tetrahedron shaped colliders
// ---------------------------------------------------------------------------

TEST(GjkComprehensive, TetrahedronShapes_Overlapping)
{
    // A rough tetrahedron mesh; two of them close enough to overlap
    const std::vector<omath::primitives::Vertex<>> tet_vbo = {
        { {  0.f,  1.f,  0.f }, {}, {} },
        { { -1.f, -1.f,  1.f }, {}, {} },
        { {  1.f, -1.f,  1.f }, {}, {} },
        { {  0.f, -1.f, -1.f }, {}, {} },
    };

    Mesh m_a{ tet_vbo, k_empty_ebo };
    Mesh m_b{ tet_vbo, k_empty_ebo };
    m_b.set_origin({ 0.5f, 0.f, 0.f });

    EXPECT_TRUE(Gjk::is_collide(Collider{ m_a }, Collider{ m_b }));
}

TEST(GjkComprehensive, TetrahedronShapes_Separated)
{
    const std::vector<omath::primitives::Vertex<>> tet_vbo = {
        { {  0.f,  1.f,  0.f }, {}, {} },
        { { -1.f, -1.f,  1.f }, {}, {} },
        { {  1.f, -1.f,  1.f }, {}, {} },
        { {  0.f, -1.f, -1.f }, {}, {} },
    };

    Mesh m_a{ tet_vbo, k_empty_ebo };
    Mesh m_b{ tet_vbo, k_empty_ebo };
    m_b.set_origin({ 3.f, 0.f, 0.f });

    EXPECT_FALSE(Gjk::is_collide(Collider{ m_a }, Collider{ m_b }));
}

// ---------------------------------------------------------------------------
// Determinism
// ---------------------------------------------------------------------------

TEST(GjkComprehensive, Deterministic_SameResultOnRepeatedCalls)
{
    const auto a = make_cube({ 0, 0, 0 });
    const auto b = make_cube({ 1.2f, 0.3f, 0.1f });
    const bool first = Gjk::is_collide(a, b);
    for (int i = 0; i < 10; ++i)
        EXPECT_EQ(Gjk::is_collide(a, b), first);
}
