//
// Created by Orange on 04/08/2026.
//
#include <gtest/gtest.h>
#include <omath/collision/bvh_tree.hpp>
#include <algorithm>
#include <random>
#include <set>

using Aabb = omath::primitives::Aabb<float>;
using BvhTree = omath::collision::BvhTree<float>;
using Ray = omath::collision::Ray<>;
using HitResult = BvhTree::HitResult;

using AabbD = omath::primitives::Aabb<double>;
using BvhTreeD = omath::collision::BvhTree<double>;

// ============================================================================
// Helper: brute-force overlap query for verification
// ============================================================================
static std::set<std::size_t> brute_force_overlaps(const std::vector<Aabb>& aabbs, const Aabb& query)
{
    std::set<std::size_t> result;
    for (std::size_t i = 0; i < aabbs.size(); ++i)
    {
        if (query.min.x <= aabbs[i].max.x && query.max.x >= aabbs[i].min.x
            && query.min.y <= aabbs[i].max.y && query.max.y >= aabbs[i].min.y
            && query.min.z <= aabbs[i].max.z && query.max.z >= aabbs[i].min.z)
            result.insert(i);
    }
    return result;
}

// ============================================================================
// Construction tests
// ============================================================================

TEST(UnitTestBvhTree, EmptyTree)
{
    const BvhTree tree;
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.node_count(), 0);
    EXPECT_TRUE(tree.query_overlaps({}).empty());
}

TEST(UnitTestBvhTree, EmptySpan)
{
    const std::vector<Aabb> empty;
    const BvhTree tree(empty);
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.node_count(), 0);
}

TEST(UnitTestBvhTree, SingleElement)
{
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}}
    };

    const BvhTree tree(aabbs);
    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(tree.node_count(), 1);

    const auto results = tree.query_overlaps({{0.5f, 0.5f, 0.5f}, {1.5f, 1.5f, 1.5f}});
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 0);

    const auto miss = tree.query_overlaps({{5.f, 5.f, 5.f}, {6.f, 6.f, 6.f}});
    EXPECT_TRUE(miss.empty());
}

TEST(UnitTestBvhTree, TwoElements)
{
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},
        {{5.f, 5.f, 5.f}, {6.f, 6.f, 6.f}},
    };

    const BvhTree tree(aabbs);
    EXPECT_FALSE(tree.empty());

    // Hit first only
    auto r = tree.query_overlaps({{-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 0);

    // Hit second only
    r = tree.query_overlaps({{5.5f, 5.5f, 5.5f}, {7.f, 7.f, 7.f}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 1);

    // Hit both
    r = tree.query_overlaps({{-1.f, -1.f, -1.f}, {10.f, 10.f, 10.f}});
    EXPECT_EQ(r.size(), 2);
}

TEST(UnitTestBvhTree, ThreeElements)
{
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},
        {{2.f, 2.f, 2.f}, {3.f, 3.f, 3.f}},
        {{10.f, 10.f, 10.f}, {11.f, 11.f, 11.f}},
    };

    const BvhTree tree(aabbs);

    const auto results = tree.query_overlaps({{0.5f, 0.5f, 0.5f}, {2.5f, 2.5f, 2.5f}});
    EXPECT_EQ(results.size(), 2);

    const auto far = tree.query_overlaps({{9.5f, 9.5f, 9.5f}, {10.5f, 10.5f, 10.5f}});
    ASSERT_EQ(far.size(), 1);
    EXPECT_EQ(far[0], 2);
}

TEST(UnitTestBvhTree, NodeCountGrowsSublinearly)
{
    // For N objects, node count should be at most 2N-1
    std::vector<Aabb> aabbs;
    for (int i = 0; i < 100; ++i)
    {
        const auto f = static_cast<float>(i) * 3.f;
        aabbs.push_back({{f, 0.f, 0.f}, {f + 1.f, 1.f, 1.f}});
    }
    const BvhTree tree(aabbs);
    EXPECT_LE(tree.node_count(), 2 * aabbs.size());
}

// ============================================================================
// Overlap query tests
// ============================================================================

TEST(UnitTestBvhTree, OverlapExactTouch)
{
    // Two boxes share exactly one face
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},
        {{1.f, 0.f, 0.f}, {2.f, 1.f, 1.f}},
    };

    const BvhTree tree(aabbs);

    // Query exactly at the shared face — should overlap both
    const auto r = tree.query_overlaps({{0.5f, 0.f, 0.f}, {1.5f, 1.f, 1.f}});
    EXPECT_EQ(r.size(), 2);
}

TEST(UnitTestBvhTree, OverlapEdgeTouch)
{
    // Query AABB edge-touches an object AABB
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},
    };

    const BvhTree tree(aabbs);

    // Touching at corner point (1,1,1)
    const auto r = tree.query_overlaps({{1.f, 1.f, 1.f}, {2.f, 2.f, 2.f}});
    EXPECT_EQ(r.size(), 1);
}

TEST(UnitTestBvhTree, OverlapQueryInsideObject)
{
    // Query is fully inside an object
    const std::vector<Aabb> aabbs = {
        {{-10.f, -10.f, -10.f}, {10.f, 10.f, 10.f}},
    };

    const BvhTree tree(aabbs);
    const auto r = tree.query_overlaps({{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 0);
}

TEST(UnitTestBvhTree, OverlapObjectInsideQuery)
{
    // Object is fully inside the query
    const std::vector<Aabb> aabbs = {
        {{4.f, 4.f, 4.f}, {5.f, 5.f, 5.f}},
    };

    const BvhTree tree(aabbs);
    const auto r = tree.query_overlaps({{0.f, 0.f, 0.f}, {10.f, 10.f, 10.f}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 0);
}

TEST(UnitTestBvhTree, OverlapMissOnSingleAxis)
{
    // Overlap on X and Y but not Z
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},
    };

    const BvhTree tree(aabbs);

    const auto r = tree.query_overlaps({{0.f, 0.f, 5.f}, {1.f, 1.f, 6.f}});
    EXPECT_TRUE(r.empty());
}

TEST(UnitTestBvhTree, OverlapNegativeCoordinates)
{
    const std::vector<Aabb> aabbs = {
        {{-5.f, -5.f, -5.f}, {-3.f, -3.f, -3.f}},
        {{-2.f, -2.f, -2.f}, {0.f, 0.f, 0.f}},
        {{1.f, 1.f, 1.f}, {3.f, 3.f, 3.f}},
    };

    const BvhTree tree(aabbs);

    const auto r = tree.query_overlaps({{-6.f, -6.f, -6.f}, {-4.f, -4.f, -4.f}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 0);
}

TEST(UnitTestBvhTree, OverlapMixedNegativePositive)
{
    const std::vector<Aabb> aabbs = {
        {{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}},
    };

    const BvhTree tree(aabbs);

    // Query spans negative and positive
    const auto r = tree.query_overlaps({{-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}});
    ASSERT_EQ(r.size(), 1);
}

TEST(UnitTestBvhTree, OverlapNoHitsAmongMany)
{
    std::vector<Aabb> aabbs;
    for (int i = 0; i < 50; ++i)
    {
        const auto f = static_cast<float>(i) * 5.f;
        aabbs.push_back({{f, 0.f, 0.f}, {f + 1.f, 1.f, 1.f}});
    }

    const BvhTree tree(aabbs);

    // Query far from all objects
    const auto r = tree.query_overlaps({{-100.f, -100.f, -100.f}, {-90.f, -90.f, -90.f}});
    EXPECT_TRUE(r.empty());
}

TEST(UnitTestBvhTree, OverlapAllObjects)
{
    std::vector<Aabb> aabbs;
    for (int i = 0; i < 64; ++i)
    {
        const auto f = static_cast<float>(i);
        aabbs.push_back({{f, f, f}, {f + 0.5f, f + 0.5f, f + 0.5f}});
    }

    const BvhTree tree(aabbs);

    const auto r = tree.query_overlaps({{-1.f, -1.f, -1.f}, {100.f, 100.f, 100.f}});
    EXPECT_EQ(r.size(), 64);
}

TEST(UnitTestBvhTree, OverlapReturnsCorrectIndices)
{
    // Specific index verification
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},     // 0
        {{10.f, 0.f, 0.f}, {11.f, 1.f, 1.f}},    // 1
        {{20.f, 0.f, 0.f}, {21.f, 1.f, 1.f}},    // 2
        {{30.f, 0.f, 0.f}, {31.f, 1.f, 1.f}},    // 3
        {{40.f, 0.f, 0.f}, {41.f, 1.f, 1.f}},    // 4
    };

    const BvhTree tree(aabbs);

    // Hit only index 2
    auto r = tree.query_overlaps({{19.5f, -1.f, -1.f}, {20.5f, 2.f, 2.f}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 2);

    // Hit only index 4
    r = tree.query_overlaps({{39.5f, -1.f, -1.f}, {40.5f, 2.f, 2.f}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 4);
}

// ============================================================================
// Spatial distribution tests
// ============================================================================

TEST(UnitTestBvhTree, ObjectsAlongXAxis)
{
    // All objects on a line along X
    std::vector<Aabb> aabbs;
    for (int i = 0; i < 20; ++i)
    {
        const auto f = static_cast<float>(i) * 4.f;
        aabbs.push_back({{f, 0.f, 0.f}, {f + 1.f, 1.f, 1.f}});
    }

    const BvhTree tree(aabbs);

    const auto r = tree.query_overlaps({{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}});
    EXPECT_EQ(r.size(), 1);

    const auto mid = tree.query_overlaps({{7.5f, -1.f, -1.f}, {8.5f, 2.f, 2.f}});
    EXPECT_EQ(mid.size(), 1);
}

TEST(UnitTestBvhTree, ObjectsAlongYAxis)
{
    std::vector<Aabb> aabbs;
    for (int i = 0; i < 20; ++i)
    {
        const auto f = static_cast<float>(i) * 4.f;
        aabbs.push_back({{0.f, f, 0.f}, {1.f, f + 1.f, 1.f}});
    }

    const BvhTree tree(aabbs);

    const auto r = tree.query_overlaps({{-1.f, 38.f, -1.f}, {2.f, 40.f, 2.f}});
    EXPECT_EQ(r.size(), 1);
}

TEST(UnitTestBvhTree, ObjectsAlongZAxis)
{
    std::vector<Aabb> aabbs;
    for (int i = 0; i < 20; ++i)
    {
        const auto f = static_cast<float>(i) * 4.f;
        aabbs.push_back({{0.f, 0.f, f}, {1.f, 1.f, f + 1.f}});
    }

    const BvhTree tree(aabbs);

    const auto r = tree.query_overlaps({{-1.f, -1.f, 38.f}, {2.f, 2.f, 40.f}});
    EXPECT_EQ(r.size(), 1);
}

TEST(UnitTestBvhTree, ObjectsInPlaneXY)
{
    // Grid in the XY plane
    std::vector<Aabb> aabbs;
    for (int x = 0; x < 10; ++x)
        for (int y = 0; y < 10; ++y)
        {
            const auto fx = static_cast<float>(x) * 3.f;
            const auto fy = static_cast<float>(y) * 3.f;
            aabbs.push_back({{fx, fy, 0.f}, {fx + 1.f, fy + 1.f, 1.f}});
        }

    const BvhTree tree(aabbs);
    EXPECT_EQ(tree.query_overlaps({{-1.f, -1.f, -1.f}, {100.f, 100.f, 2.f}}).size(), 100);

    // Single cell query
    const auto r = tree.query_overlaps({{0.f, 0.f, 0.f}, {0.5f, 0.5f, 0.5f}});
    EXPECT_EQ(r.size(), 1);
}

TEST(UnitTestBvhTree, ClusteredObjects)
{
    // Two clusters far apart
    std::vector<Aabb> aabbs;
    for (int i = 0; i < 25; ++i)
    {
        const auto f = static_cast<float>(i) * 0.5f;
        aabbs.push_back({{f, f, f}, {f + 0.4f, f + 0.4f, f + 0.4f}});
    }
    for (int i = 0; i < 25; ++i)
    {
        const auto f = 100.f + static_cast<float>(i) * 0.5f;
        aabbs.push_back({{f, f, f}, {f + 0.4f, f + 0.4f, f + 0.4f}});
    }

    const BvhTree tree(aabbs);

    // Query near first cluster
    const auto r1 = tree.query_overlaps({{-1.f, -1.f, -1.f}, {15.f, 15.f, 15.f}});
    EXPECT_EQ(r1.size(), 25);

    // Query near second cluster
    const auto r2 = tree.query_overlaps({{99.f, 99.f, 99.f}, {115.f, 115.f, 115.f}});
    EXPECT_EQ(r2.size(), 25);

    // Query between clusters — should find nothing
    const auto gap = tree.query_overlaps({{50.f, 50.f, 50.f}, {60.f, 60.f, 60.f}});
    EXPECT_TRUE(gap.empty());
}

TEST(UnitTestBvhTree, OverlappingObjects)
{
    // Objects that overlap each other
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {2.f, 2.f, 2.f}},
        {{1.f, 1.f, 1.f}, {3.f, 3.f, 3.f}},
        {{1.5f, 1.5f, 1.5f}, {4.f, 4.f, 4.f}},
    };

    const BvhTree tree(aabbs);

    // Query at the overlap region of all three
    const auto r = tree.query_overlaps({{1.5f, 1.5f, 1.5f}, {2.f, 2.f, 2.f}});
    EXPECT_EQ(r.size(), 3);
}

TEST(UnitTestBvhTree, IdenticalObjects)
{
    // All objects at the same position
    std::vector<Aabb> aabbs;
    for (int i = 0; i < 10; ++i)
        aabbs.push_back({{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}});

    const BvhTree tree(aabbs);

    const auto r = tree.query_overlaps({{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}});
    EXPECT_EQ(r.size(), 10);
}

TEST(UnitTestBvhTree, DegenerateThickPlanes)
{
    // Very flat AABBs (thickness ~0 in one axis)
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {10.f, 10.f, 0.001f}},
        {{0.f, 0.f, 5.f}, {10.f, 10.f, 5.001f}},
    };

    const BvhTree tree(aabbs);

    const auto r = tree.query_overlaps({{0.f, 0.f, -0.01f}, {10.f, 10.f, 0.01f}});
    ASSERT_EQ(r.size(), 1);
}

TEST(UnitTestBvhTree, VaryingSizes)
{
    // Objects of wildly different sizes
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {0.01f, 0.01f, 0.01f}},         // tiny
        {{-500.f, -500.f, -500.f}, {500.f, 500.f, 500.f}}, // huge
        {{10.f, 10.f, 10.f}, {11.f, 11.f, 11.f}},          // normal
    };

    const BvhTree tree(aabbs);

    // The huge box should overlap almost any query
    auto r = tree.query_overlaps({{200.f, 200.f, 200.f}, {201.f, 201.f, 201.f}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 1);

    // Query at origin hits the tiny and the huge
    r = tree.query_overlaps({{-0.1f, -0.1f, -0.1f}, {0.1f, 0.1f, 0.1f}});
    EXPECT_EQ(r.size(), 2);
}

// ============================================================================
// Ray query tests
// ============================================================================

TEST(UnitTestBvhTree, RayQueryBasic)
{
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},
        {{5.f, 0.f, 0.f}, {6.f, 1.f, 1.f}},
        {{0.f, 5.f, 0.f}, {1.f, 6.f, 1.f}},
    };

    const BvhTree tree(aabbs);

    Ray ray;
    ray.start = {-1.f, 0.5f, 0.5f};
    ray.end = {10.f, 0.5f, 0.5f};
    ray.infinite_length = true;

    const auto hits = tree.query_ray(ray);
    EXPECT_GE(hits.size(), 2);

    if (hits.size() >= 2)
        EXPECT_LE(hits[0].distance_sqr, hits[1].distance_sqr);
}

TEST(UnitTestBvhTree, RayQueryMissesAll)
{
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},
        {{5.f, 0.f, 0.f}, {6.f, 1.f, 1.f}},
    };

    const BvhTree tree(aabbs);

    // Ray above everything
    Ray ray;
    ray.start = {-1.f, 100.f, 0.5f};
    ray.end = {10.f, 100.f, 0.5f};
    ray.infinite_length = true;

    const auto hits = tree.query_ray(ray);
    EXPECT_TRUE(hits.empty());
}

TEST(UnitTestBvhTree, RayQueryAlongY)
{
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},
        {{0.f, 5.f, 0.f}, {1.f, 6.f, 1.f}},
        {{0.f, 10.f, 0.f}, {1.f, 11.f, 1.f}},
    };

    const BvhTree tree(aabbs);

    Ray ray;
    ray.start = {0.5f, -1.f, 0.5f};
    ray.end = {0.5f, 20.f, 0.5f};
    ray.infinite_length = true;

    const auto hits = tree.query_ray(ray);
    EXPECT_EQ(hits.size(), 3);

    // Verify sorted by distance
    for (std::size_t i = 1; i < hits.size(); ++i)
        EXPECT_LE(hits[i - 1].distance_sqr, hits[i].distance_sqr);
}

TEST(UnitTestBvhTree, RayQueryAlongZ)
{
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},
        {{0.f, 0.f, 10.f}, {1.f, 1.f, 11.f}},
    };

    const BvhTree tree(aabbs);

    Ray ray;
    ray.start = {0.5f, 0.5f, -5.f};
    ray.end = {0.5f, 0.5f, 20.f};
    ray.infinite_length = true;

    const auto hits = tree.query_ray(ray);
    EXPECT_EQ(hits.size(), 2);
}

TEST(UnitTestBvhTree, RayQueryDiagonal)
{
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},
        {{5.f, 5.f, 5.f}, {6.f, 6.f, 6.f}},
        {{10.f, 10.f, 10.f}, {11.f, 11.f, 11.f}},
    };

    const BvhTree tree(aabbs);

    // Diagonal ray through all three
    Ray ray;
    ray.start = {-1.f, -1.f, -1.f};
    ray.end = {15.f, 15.f, 15.f};
    ray.infinite_length = true;

    const auto hits = tree.query_ray(ray);
    EXPECT_EQ(hits.size(), 3);
}

TEST(UnitTestBvhTree, RayQueryOnEmptyTree)
{
    const BvhTree tree;

    Ray ray;
    ray.start = {0.f, 0.f, 0.f};
    ray.end = {10.f, 0.f, 0.f};
    ray.infinite_length = true;

    const auto hits = tree.query_ray(ray);
    EXPECT_TRUE(hits.empty());
}

TEST(UnitTestBvhTree, RayQuerySortedByDistance)
{
    // Many boxes along a line
    std::vector<Aabb> aabbs;
    for (int i = 0; i < 20; ++i)
    {
        const auto f = static_cast<float>(i) * 3.f;
        aabbs.push_back({{f, 0.f, 0.f}, {f + 1.f, 1.f, 1.f}});
    }

    const BvhTree tree(aabbs);

    Ray ray;
    ray.start = {-1.f, 0.5f, 0.5f};
    ray.end = {100.f, 0.5f, 0.5f};
    ray.infinite_length = true;

    const auto hits = tree.query_ray(ray);
    EXPECT_EQ(hits.size(), 20);

    for (std::size_t i = 1; i < hits.size(); ++i)
        EXPECT_LE(hits[i - 1].distance_sqr, hits[i].distance_sqr);
}

// ============================================================================
// Brute-force verification tests
// ============================================================================

TEST(UnitTestBvhTree, BruteForceVerificationGrid)
{
    std::vector<Aabb> aabbs;
    for (int x = 0; x < 10; ++x)
        for (int y = 0; y < 10; ++y)
            for (int z = 0; z < 10; ++z)
            {
                const auto fx = static_cast<float>(x) * 3.f;
                const auto fy = static_cast<float>(y) * 3.f;
                const auto fz = static_cast<float>(z) * 3.f;
                aabbs.push_back({{fx, fy, fz}, {fx + 1.f, fy + 1.f, fz + 1.f}});
            }

    const BvhTree tree(aabbs);

    // Test several queries and compare to brute force
    const std::vector<Aabb> queries = {
        {{0.f, 0.f, 0.f}, {1.5f, 1.5f, 1.5f}},
        {{-1.f, -1.f, -1.f}, {100.f, 100.f, 100.f}},
        {{13.f, 13.f, 13.f}, {14.f, 14.f, 14.f}},
        {{-50.f, -50.f, -50.f}, {-40.f, -40.f, -40.f}},
        {{5.5f, 5.5f, 5.5f}, {7.5f, 7.5f, 7.5f}},
    };

    for (const auto& q : queries)
    {
        const auto bvh_results = tree.query_overlaps(q);
        const auto brute_results = brute_force_overlaps(aabbs, q);

        const std::set<std::size_t> bvh_set(bvh_results.begin(), bvh_results.end());
        EXPECT_EQ(bvh_set, brute_results)
            << "Mismatch for query [(" << q.min.x << "," << q.min.y << "," << q.min.z
            << ") -> (" << q.max.x << "," << q.max.y << "," << q.max.z << ")]";
    }
}

TEST(UnitTestBvhTree, BruteForceVerificationRandom)
{
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> pos_dist(-50.f, 50.f);
    std::uniform_real_distribution<float> size_dist(0.5f, 3.f);

    std::vector<Aabb> aabbs;
    for (int i = 0; i < 200; ++i)
    {
        const auto x = pos_dist(rng);
        const auto y = pos_dist(rng);
        const auto z = pos_dist(rng);
        const auto sx = size_dist(rng);
        const auto sy = size_dist(rng);
        const auto sz = size_dist(rng);
        aabbs.push_back({{x, y, z}, {x + sx, y + sy, z + sz}});
    }

    const BvhTree tree(aabbs);

    // Run 50 random queries
    for (int i = 0; i < 50; ++i)
    {
        const auto qx = pos_dist(rng);
        const auto qy = pos_dist(rng);
        const auto qz = pos_dist(rng);
        const auto qsx = size_dist(rng);
        const auto qsy = size_dist(rng);
        const auto qsz = size_dist(rng);
        const Aabb query = {{qx, qy, qz}, {qx + qsx, qy + qsy, qz + qsz}};

        const auto bvh_results = tree.query_overlaps(query);
        const auto brute_results = brute_force_overlaps(aabbs, query);

        const std::set<std::size_t> bvh_set(bvh_results.begin(), bvh_results.end());
        EXPECT_EQ(bvh_set, brute_results) << "Mismatch on random query iteration " << i;
    }
}

// ============================================================================
// Large dataset tests
// ============================================================================

TEST(UnitTestBvhTree, LargeGridDataset)
{
    std::vector<Aabb> aabbs;
    for (int x = 0; x < 10; ++x)
        for (int y = 0; y < 10; ++y)
            for (int z = 0; z < 10; ++z)
            {
                const auto fx = static_cast<float>(x) * 3.f;
                const auto fy = static_cast<float>(y) * 3.f;
                const auto fz = static_cast<float>(z) * 3.f;
                aabbs.push_back({{fx, fy, fz}, {fx + 1.f, fy + 1.f, fz + 1.f}});
            }

    const BvhTree tree(aabbs);
    EXPECT_FALSE(tree.empty());

    const auto results = tree.query_overlaps({{0.f, 0.f, 0.f}, {1.5f, 1.5f, 1.5f}});
    EXPECT_EQ(results.size(), 1);

    const auto all_results = tree.query_overlaps({{-1.f, -1.f, -1.f}, {100.f, 100.f, 100.f}});
    EXPECT_EQ(all_results.size(), 1000);
}

TEST(UnitTestBvhTree, FiveThousandObjects)
{
    std::vector<Aabb> aabbs;
    for (int i = 0; i < 5000; ++i)
    {
        const auto f = static_cast<float>(i) * 2.f;
        aabbs.push_back({{f, 0.f, 0.f}, {f + 1.f, 1.f, 1.f}});
    }

    const BvhTree tree(aabbs);
    EXPECT_FALSE(tree.empty());

    // Query that should hit exactly 1
    const auto r = tree.query_overlaps({{0.f, 0.f, 0.f}, {0.5f, 0.5f, 0.5f}});
    EXPECT_EQ(r.size(), 1);

    // Query that misses
    const auto miss = tree.query_overlaps({{-100.f, -100.f, -100.f}, {-90.f, -90.f, -90.f}});
    EXPECT_TRUE(miss.empty());
}

// ============================================================================
// Double precision tests
// ============================================================================

TEST(UnitTestBvhTree, DoublePrecision)
{
    const std::vector<AabbD> aabbs = {
        {{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}},
        {{5.0, 5.0, 5.0}, {6.0, 6.0, 6.0}},
        {{10.0, 10.0, 10.0}, {11.0, 11.0, 11.0}},
    };

    const BvhTreeD tree(aabbs);
    EXPECT_FALSE(tree.empty());

    const auto r = tree.query_overlaps({{0.5, 0.5, 0.5}, {1.5, 1.5, 1.5}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 0);

    const auto r2 = tree.query_overlaps({{4.5, 4.5, 4.5}, {5.5, 5.5, 5.5}});
    ASSERT_EQ(r2.size(), 1);
    EXPECT_EQ(r2[0], 1);
}

TEST(UnitTestBvhTree, DoublePrecisionLargeCoordinates)
{
    const std::vector<AabbD> aabbs = {
        {{1e10, 1e10, 1e10}, {1e10 + 1.0, 1e10 + 1.0, 1e10 + 1.0}},
        {{-1e10, -1e10, -1e10}, {-1e10 + 1.0, -1e10 + 1.0, -1e10 + 1.0}},
    };

    const BvhTreeD tree(aabbs);

    const auto r = tree.query_overlaps({{1e10 - 0.5, 1e10 - 0.5, 1e10 - 0.5},
                                         {1e10 + 0.5, 1e10 + 0.5, 1e10 + 0.5}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 0);
}

// ============================================================================
// Edge case tests
// ============================================================================

TEST(UnitTestBvhTree, ZeroSizeQuery)
{
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},
    };

    const BvhTree tree(aabbs);

    // Point query inside the box
    const auto r = tree.query_overlaps({{0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}});
    EXPECT_EQ(r.size(), 1);

    // Point query outside the box
    const auto miss = tree.query_overlaps({{5.f, 5.f, 5.f}, {5.f, 5.f, 5.f}});
    EXPECT_TRUE(miss.empty());
}

TEST(UnitTestBvhTree, ZeroSizeObjects)
{
    // Point-like AABBs
    const std::vector<Aabb> aabbs = {
        {{1.f, 1.f, 1.f}, {1.f, 1.f, 1.f}},
        {{5.f, 5.f, 5.f}, {5.f, 5.f, 5.f}},
    };

    const BvhTree tree(aabbs);

    const auto r = tree.query_overlaps({{0.f, 0.f, 0.f}, {2.f, 2.f, 2.f}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 0);
}

TEST(UnitTestBvhTree, NoDuplicateResults)
{
    std::vector<Aabb> aabbs;
    for (int i = 0; i < 50; ++i)
    {
        const auto f = static_cast<float>(i) * 2.f;
        aabbs.push_back({{f, 0.f, 0.f}, {f + 1.f, 1.f, 1.f}});
    }

    const BvhTree tree(aabbs);

    const auto r = tree.query_overlaps({{-1.f, -1.f, -1.f}, {200.f, 2.f, 2.f}});

    // Check for duplicates
    const std::set<std::size_t> unique_results(r.begin(), r.end());
    EXPECT_EQ(unique_results.size(), r.size());
    EXPECT_EQ(r.size(), 50);
}

TEST(UnitTestBvhTree, LargeSpread)
{
    // Objects with huge gaps between them
    const std::vector<Aabb> aabbs = {
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}},
        {{1000.f, 0.f, 0.f}, {1001.f, 1.f, 1.f}},
        {{-1000.f, 0.f, 0.f}, {-999.f, 1.f, 1.f}},
        {{0.f, 1000.f, 0.f}, {1.f, 1001.f, 1.f}},
        {{0.f, -1000.f, 0.f}, {1.f, -999.f, 1.f}},
    };

    const BvhTree tree(aabbs);

    auto r = tree.query_overlaps({{999.f, -1.f, -1.f}, {1002.f, 2.f, 2.f}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 1);

    r = tree.query_overlaps({{-1001.f, -1.f, -1.f}, {-998.f, 2.f, 2.f}});
    ASSERT_EQ(r.size(), 1);
    EXPECT_EQ(r[0], 2);
}

TEST(UnitTestBvhTree, AllObjectsSameCenter)
{
    // All AABBs centered at origin but different sizes
    std::vector<Aabb> aabbs;
    for (int i = 1; i <= 10; ++i)
    {
        const auto s = static_cast<float>(i);
        aabbs.push_back({{-s, -s, -s}, {s, s, s}});
    }

    const BvhTree tree(aabbs);

    // Small query at origin should hit all
    const auto r = tree.query_overlaps({{-0.1f, -0.1f, -0.1f}, {0.1f, 0.1f, 0.1f}});
    EXPECT_EQ(r.size(), 10);

    // Query touching only the largest box
    const auto r2 = tree.query_overlaps({{9.5f, 9.5f, 9.5f}, {10.5f, 10.5f, 10.5f}});
    ASSERT_EQ(r2.size(), 1);
    EXPECT_EQ(r2[0], 9);
}

TEST(UnitTestBvhTree, MultipleQueriesSameTree)
{
    std::vector<Aabb> aabbs;
    for (int i = 0; i < 100; ++i)
    {
        const auto f = static_cast<float>(i) * 2.f;
        aabbs.push_back({{f, 0.f, 0.f}, {f + 1.f, 1.f, 1.f}});
    }

    const BvhTree tree(aabbs);

    // Run many queries on the same tree
    for (int i = 0; i < 100; ++i)
    {
        const auto f = static_cast<float>(i) * 2.f;
        const auto r = tree.query_overlaps({{f, 0.f, 0.f}, {f + 1.f, 1.f, 1.f}});
        ASSERT_GE(r.size(), 1) << "Query for object " << i << " should find at least itself";
    }
}
