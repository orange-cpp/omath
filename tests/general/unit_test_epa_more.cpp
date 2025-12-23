#include "omath/collision/epa_algorithm.hpp"
#include "omath/collision/simplex.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>

using Vector3f = omath::Vector3<float>;

// Minimal collider interface matching Epa's expectations
struct DegenerateCollider
{
    using VectorType = Vector3f;
    // returns furthest point along dir
    VectorType find_abs_furthest_vertex_position(const VectorType& dir) const noexcept
    {
        // Always return points on a small circle in XY plane so some faces become degenerate
        if (dir.x > 0.5f) return {0.01f, 0.f, 0.f};
        if (dir.x < -0.5f) return {-0.01f, 0.f, 0.f};
        if (dir.y > 0.5f) return {0.f, 0.01f, 0.f};
        if (dir.y < -0.5f) return {0.f, -0.01f, 0.f};
        return {0.f, 0.f, 0.01f};
    }
};

using Epa = omath::collision::Epa<DegenerateCollider>;
using Simplex = omath::collision::Simplex<Vector3f>;

TEST(EpaExtra, DegenerateFaceHandled)
{
    // Prepare a simplex with near-collinear points to force degenerate face handling
    Simplex s;
    s = { Vector3f{0.01f, 0.f, 0.f}, Vector3f{0.02f, 0.f, 0.f}, Vector3f{0.03f, 0.f, 0.f}, Vector3f{0.0f, 0.0f, 0.01f} };

    constexpr DegenerateCollider a;
    constexpr DegenerateCollider b;
    Epa::Params params;
    params.max_iterations = 4;
    params.tolerance = 1e-6f;

    const auto result = Epa::solve(a, b, s, params);

    // The algorithm should either return a valid result or gracefully exit (not crash)
    if (result)
    {
        EXPECT_TRUE(std::isfinite(result->depth));
        EXPECT_TRUE(std::isfinite(result->normal.x));
    }
    else
    {
        SUCCEED() << "EPA returned nullopt for degenerate input (acceptable)";
    }
}
