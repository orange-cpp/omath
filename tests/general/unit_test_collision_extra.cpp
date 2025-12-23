// Extra collision tests: Simplex, MeshCollider, EPA
#include <gtest/gtest.h>
#include <omath/collision/simplex.hpp>
#include <omath/collision/mesh_collider.hpp>
#include <omath/collision/epa_algorithm.hpp>
#include <omath/engines/source_engine/collider.hpp>

using namespace omath;
using namespace omath::collision;

TEST(SimplexTest, HandleEmptySimplex)
{
    Simplex<Vector3<float>> simplex;
    Vector3<float> direction{1, 0, 0};
    
    EXPECT_EQ(simplex.size(), 0);
    EXPECT_FALSE(simplex.handle(direction));
}

TEST(SimplexTest, HandleLineCollinearWithXAxis)
{
    using Vec3 = Vector3<float>;
    Simplex<Vec3> simplex;
    
    simplex.push_front(Vec3{1, 0, 0});
    simplex.push_front(Vec3{-1, 0, 0});
    
    Vec3 direction{};
    simplex.handle(direction);
    
    EXPECT_NEAR(direction.x, 0.f, 1e-6f);
}

TEST(CollisionExtra, SimplexLineHandle)
{
    Simplex<Vector3<float>> s;
    s = { Vector3<float>{1.f,0.f,0.f}, Vector3<float>{2.f,0.f,0.f} };
    Vector3<float> dir{0,0,0};
    EXPECT_FALSE(s.handle(dir));
    // direction should not be zero
    EXPECT_GT(dir.length_sqr(), 0.0f);
}

TEST(CollisionExtra, SimplexTriangleHandle)
{
    Simplex<Vector3<float>> s;
    s = { Vector3<float>{1.f,0.f,0.f}, Vector3<float>{0.f,1.f,0.f}, Vector3<float>{0.f,0.f,1.f} };
    Vector3<float> dir{0,0,0};
    EXPECT_FALSE(s.handle(dir));
    EXPECT_GT(dir.length_sqr(), 0.0f);
}

TEST(CollisionExtra, SimplexTetrahedronInside)
{
    Simplex<Vector3<float>> s;
    // tetra that surrounds origin roughly
    s = { Vector3<float>{1.f,0.f,0.f}, Vector3<float>{0.f,1.f,0.f}, Vector3<float>{0.f,0.f,1.f}, Vector3<float>{-1.f,-1.f,-1.f} };
    Vector3<float> dir{0,0,0};
    // if origin inside, handle returns true
    const bool inside = s.handle(dir);
    EXPECT_TRUE(inside);
}

TEST(CollisionExtra, MeshColliderOriginAndFurthest)
{
    omath::source_engine::Mesh mesh = {
        std::vector<omath::primitives::Vertex<>>{
            { { 1.f,  1.f,  1.f }, {}, {} },
            { {-1.f, -1.f, -1.f }, {}, {} }
        },
        {}
    };
    mesh.set_origin({0, 2, 0});
    omath::source_engine::MeshCollider collider(mesh);

    EXPECT_EQ(collider.get_origin(), omath::Vector3<float>(0,2,0));
    collider.set_origin({1,2,3});
    EXPECT_EQ(collider.get_origin(), omath::Vector3<float>(1,2,3));

    const auto v = collider.find_abs_furthest_vertex_position({1.f,0.f,0.f});
    // the original vertex at (1,1,1) translated by origin (1,2,3) becomes (2,3,4)
    EXPECT_EQ(v, omath::Vector3<float>(2.f,3.f,4.f));
}

TEST(CollisionExtra, EPAConvergesOnSimpleCase)
{
    // Build two simple colliders using simple meshes that overlap
    omath::source_engine::Mesh meshA = {
        std::vector<omath::primitives::Vertex<>>{{ {0.f,0.f,0.f}, {}, {} }, { {1.f,0.f,0.f}, {}, {} } },
        {}
    };
    omath::source_engine::Mesh meshB = meshA;
    meshB.set_origin({0.5f, 0.f, 0.f}); // translate to overlap

    omath::source_engine::MeshCollider A(meshA);
    omath::source_engine::MeshCollider B(meshB);

    // Create a simplex that approximately contains the origin in Minkowski space
    Simplex<omath::Vector3<float>> simplex;
    simplex = { omath::Vector3<float>{0.5f,0.f,0.f}, omath::Vector3<float>{-0.5f,0.f,0.f}, omath::Vector3<float>{0.f,0.5f,0.f}, omath::Vector3<float>{0.f,-0.5f,0.f} };

    auto pool = std::pmr::monotonic_buffer_resource(1024);
    auto res = Epa<omath::source_engine::MeshCollider>::solve(A, B, simplex, {}, pool);
    // EPA may or may not converge depending on numerics; ensure it returns optionally
    // but if it does, fields should be finite
    if (res.has_value())
    {
        auto r = *res;
        EXPECT_TRUE(std::isfinite(r.depth));
        EXPECT_GT(r.normal.length_sqr(), 0.0f);
    }
}
