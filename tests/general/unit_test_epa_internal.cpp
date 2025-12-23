#include "omath/collision/epa_algorithm.hpp"
#include "omath/collision/simplex.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>

using Vector3f = omath::Vector3<float>;

// Dummy collider type that exposes VectorType and returns small offsets
struct DummyCollider
{
    using VectorType = Vector3f;
    [[nodiscard]]
    static VectorType find_abs_furthest_vertex_position(const VectorType& dir) noexcept
    {
        // map direction to a small point so support_point is finite
        return Vector3f{dir.x * 0.01f, dir.y * 0.01f, dir.z * 0.01f};
    }
};

using EpaDummy = omath::collision::Epa<DummyCollider>;
using Simplex = omath::collision::Simplex<Vector3f>;

TEST(EpaInternal, SolveHandlesSmallPolytope)
{
    // Create a simplex that is nearly degenerate but valid for solve
    Simplex s;
    s = { Vector3f{0.01f, 0.f, 0.f}, Vector3f{0.f, 0.01f, 0.f}, Vector3f{0.f, 0.f, 0.01f}, Vector3f{-0.01f, -0.01f, -0.01f} };

    constexpr DummyCollider a;
    constexpr DummyCollider b;
    EpaDummy::Params params;
    params.max_iterations = 16;
    params.tolerance = 1e-6f;

    const auto result = EpaDummy::solve(a, b, s, params);

    // Should either return a valid result or gracefully return nullopt
    if (result)
    {
        EXPECT_TRUE(std::isfinite(result->depth));
        EXPECT_TRUE(std::isfinite(result->normal.x));
        EXPECT_GT(result->num_faces, 0);
    }
    else
    {
        SUCCEED() << "Epa::solve returned nullopt for small polytope (acceptable)";
    }
}
