//
// Revised unit‑test suite for LineTracer (segment‑based Möller–Trumbore)
// Pure ASCII: avoids non‑standard characters that MSVC rejects.
//
#include "gtest/gtest.h"
#include "omath/collision/line_tracer.hpp"
#include "omath/triangle.hpp"
#include "omath/vector3.hpp"
#include <cmath>

using namespace omath;
using namespace omath::collision;

using Vec3 = Vector3<float>;

namespace {

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
    LineTracerFixture()
        : triangle({0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}) {}

    Triangle<Vec3> triangle;
};

// -----------------------------------------------------------------------------
// Data‑driven tests for CanTraceLine
// -----------------------------------------------------------------------------
struct TraceCase
{
    Ray  ray;
    bool expected_clear; // true => segment does NOT hit the triangle
};

class CanTraceLineParam : public LineTracerFixture,
                          public ::testing::WithParamInterface<TraceCase> {};

TEST_P(CanTraceLineParam, VariousRays)
{
    const auto& p = GetParam();
    EXPECT_EQ(LineTracer::CanTraceLine(p.ray, triangle), p.expected_clear);
}

INSTANTIATE_TEST_SUITE_P(
    BasicScenarios,
    CanTraceLineParam,
    ::testing::Values(
        TraceCase{Ray{{ 0.3f, 0.3f, -1.f},{ 0.3f, 0.3f, 1.f}}, false}, // hit through centre
        TraceCase{Ray{{ 0.3f, 0.3f,  1.f},{ 0.3f, 0.3f, 2.f}},  true}, // parallel above
        TraceCase{Ray{{ 0.3f, 0.3f,  0.f},{ 0.3f, 0.3f,-1.f}}, true}, // starts inside, goes away
        TraceCase{Ray{{ 2.0f, 2.0f, -1.f},{ 2.0f, 2.0f, 1.f}}, true}, // misses entirely
        TraceCase{Ray{{-1.0f,-1.0f,  0.f},{ 1.5f, 1.5f, 0.f}},true}, // lies in plane, outside tri
        TraceCase{Ray{{-1.0f,-1.0f, -1.f},{ 0.0f, 0.0f, 0.f}}, true}, // endpoint on vertex
        TraceCase{Ray{{-1.0f, 0.0f, -1.f},{ 0.5f, 0.0f, 0.f}}, true}  // endpoint on edge
    )
);

// -----------------------------------------------------------------------------
// Validate that the reported hit point is correct for a genuine intersection.
// -----------------------------------------------------------------------------
TEST_F(LineTracerFixture, HitPointCorrect)
{
    Ray ray{{0.3f, 0.3f, -1.f}, {0.3f, 0.3f, 1.f}};
    Vec3 expected{0.3f, 0.3f, 0.f};

    Vec3 hit = LineTracer::GetRayHitPoint(ray, triangle);
    ASSERT_FALSE(VecEqual(hit, ray.end));
    EXPECT_TRUE(VecEqual(hit, expected));
}

// -----------------------------------------------------------------------------
// Triangle far beyond the ray should not block.
// -----------------------------------------------------------------------------
TEST_F(LineTracerFixture, DistantTriangleClear)
{
    Ray short_ray{{0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}};
    Triangle<Vec3> distant{{1000.f,1000.f,1000.f},
                           {1001.f,1000.f,1000.f},
                           {1000.f,1001.f,1000.f}};

    EXPECT_TRUE(LineTracer::CanTraceLine(short_ray, distant));
}

} // namespace
