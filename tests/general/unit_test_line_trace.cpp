//
// Revised unit‑test suite for LineTracer (segment‑based Möller–Trumbore)
// Pure ASCII: avoids non‑standard characters that MSVC rejects.
//
#include <vector>
#include "gtest/gtest.h"
#include <cmath>
import omath.collision.line_trace;
using namespace omath;
using namespace omath::collision;

using Vec3 = Vector3<float>;

namespace
{

    // -----------------------------------------------------------------------------
    // Constants & helpers
    // -----------------------------------------------------------------------------
    constexpr float kTol = 1e-5f;

    bool VecEqual(const Vec3& a, const Vec3& b, float tol = kTol)
    {
        return std::fabs(a.x - b.x) < tol &&
               std::fabs(a.y - b.y) < tol &&
               std::fabs(a.z - b.z) < tol;
    }

    // -----------------------------------------------------------------------------
    // Fixture with one canonical right‑angled triangle in the XY plane.
    // -----------------------------------------------------------------------------
    class LineTracerFixture : public ::testing::Test
    {
    protected:
        LineTracerFixture() :
            triangle({0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f})
        {
        }

        Triangle<Vec3> triangle;
    };

    // -----------------------------------------------------------------------------
    // Data‑driven tests for CanTraceLine
    // -----------------------------------------------------------------------------
    struct TraceCase
    {
        Ray ray;
        bool expected_clear; // true => segment does NOT hit the triangle
    };

    class CanTraceLineParam : public LineTracerFixture,
                              public ::testing::WithParamInterface<TraceCase>
    {
    };

    TEST_P(CanTraceLineParam, VariousRays)
    {
        const auto& p = GetParam();
        EXPECT_EQ(LineTracer::can_trace_line(p.ray, triangle), p.expected_clear);
    }

    INSTANTIATE_TEST_SUITE_P(
            BasicScenarios,
            CanTraceLineParam,
            ::testing::Values(
                TraceCase{Ray{{ 0.3f, 0.3f, -1.f},{ 0.3f, 0.3f, 1.f}}, false}, // hit through centre
                TraceCase{Ray{{ 0.3f, 0.3f, 1.f},{ 0.3f, 0.3f, 2.f}}, true}, // parallel above
                TraceCase{Ray{{ 0.3f, 0.3f, 0.f},{ 0.3f, 0.3f,-1.f}}, true}, // starts inside, goes away
                TraceCase{Ray{{ 2.0f, 2.0f, -1.f},{ 2.0f, 2.0f, 1.f}}, true}, // misses entirely
                TraceCase{Ray{{-1.0f,-1.0f, 0.f},{ 1.5f, 1.5f, 0.f}},true}, // lies in plane, outside tri
                TraceCase{Ray{{-1.0f,-1.0f, -1.f},{ 0.0f, 0.0f, 0.f}}, true}, // endpoint on vertex
                TraceCase{Ray{{-1.0f, 0.0f, -1.f},{ 0.5f, 0.0f, 0.f}}, true} // endpoint on edge
            )
            );

    // -----------------------------------------------------------------------------
    // Validate that the reported hit point is correct for a genuine intersection.
    // -----------------------------------------------------------------------------
    TEST_F(LineTracerFixture, HitPointCorrect)
    {
        constexpr Ray ray{{0.3f, 0.3f, -1.f}, {0.3f, 0.3f, 1.f}};
        constexpr Vec3 expected{0.3f, 0.3f, 0.f};

        const Vec3 hit = LineTracer::get_ray_hit_point(ray, triangle);
        ASSERT_FALSE(VecEqual(hit, ray.end));
        EXPECT_TRUE(VecEqual(hit, expected));
    }

    // -----------------------------------------------------------------------------
    // Triangle far beyond the ray should not block.
    // -----------------------------------------------------------------------------
    TEST_F(LineTracerFixture, DistantTriangleClear)
    {
        constexpr Ray short_ray{{0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}};
        constexpr Triangle<Vec3> distant{{1000.f, 1000.f, 1000.f},
                                         {1001.f, 1000.f, 1000.f},
                                         {1000.f, 1001.f, 1000.f}};

        EXPECT_TRUE(LineTracer::can_trace_line(short_ray, distant));
    }

    TEST(unit_test_unity_engine, CantHit)
    {
        constexpr omath::Triangle<Vector3<float>> triangle{{2, 0, 0}, {2, 2, 0}, {2, 2, 2}};

        constexpr Ray ray{{}, {1.0, 0, 0}, false};

        EXPECT_TRUE(omath::collision::LineTracer::can_trace_line(ray, triangle));
    }
    TEST(unit_test_unity_engine, CanHit)
    {
        constexpr omath::Triangle<Vector3<float>> triangle{{2, 0, 0}, {2, 2, 0}, {2, 2, 2}};

        constexpr Ray ray{{}, {2.1, 0, 0}, false};
        EXPECT_FALSE(omath::collision::LineTracer::can_trace_line(ray, triangle));
    }
} // namespace
