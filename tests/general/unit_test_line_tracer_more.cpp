#include "omath/collision/line_tracer.hpp"
#include "omath/linear_algebra/triangle.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>

using omath::Vector3;
using omath::collision::Ray;
using omath::collision::LineTracer;
using Triangle3 = omath::Triangle<Vector3<float>>;

TEST(LineTracerMore, ParallelRayReturnsEnd)
{
    // Ray parallel to triangle plane: construct triangle in XY plane and ray along X axis
    constexpr Triangle3 tri(Vector3<float>{0.f,0.f,0.f}, Vector3<float>{1.f,0.f,0.f}, Vector3<float>{0.f,1.f,0.f});
    Ray ray; ray.start = {0.f,0.f,1.f}; ray.end = {1.f,0.f,1.f};

    const auto hit = LineTracer<>::get_ray_hit_point(ray, tri);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerMore, UOutOfRangeReturnsEnd)
{
    // Construct a ray that misses due to u < 0
    constexpr Triangle3 tri(Vector3<float>{0.f,0.f,0.f}, Vector3<float>{1.f,0.f,0.f}, Vector3<float>{0.f,1.f,0.f});
    Ray ray; ray.start = {-1.f,-1.f,-1.f}; ray.end = {-0.5f,-1.f,1.f};

    const auto hit = LineTracer<>::get_ray_hit_point(ray, tri);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerMore, VOutOfRangeReturnsEnd)
{
    // Construct ray that has v < 0
    constexpr Triangle3 tri(Vector3<float>{0.f,0.f,0.f}, Vector3<float>{1.f,0.f,0.f}, Vector3<float>{0.f,1.f,0.f});
    Ray ray; ray.start = {2.f,2.f,-1.f}; ray.end = {2.f,2.f,1.f};

    const auto hit = LineTracer<>::get_ray_hit_point(ray, tri);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerMore, THitTooSmallReturnsEnd)
{
    constexpr Triangle3 tri(Vector3<float>{0.f,0.f,0.f}, Vector3<float>{1.f,0.f,0.f}, Vector3<float>{0.f,1.f,0.f});
    Ray ray; ray.start = {0.f,0.f,0.0000000001f}; ray.end = {0.f,0.f,1.f};

    const auto hit = LineTracer<>::get_ray_hit_point(ray, tri);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerMore, THitGreaterThanOneReturnsEnd)
{
    constexpr Triangle3 tri(Vector3<float>{0.f,0.f,0.f}, Vector3<float>{1.f,0.f,0.f}, Vector3<float>{0.f,1.f,0.f});
    // Choose a ray and compute t_hit locally to assert consistency
    Ray ray; ray.start = {0.f,0.f,-1.f}; ray.end = {0.f,0.f,-0.5f};

    const auto hit = LineTracer<>::get_ray_hit_point(ray, tri);

    constexpr float k_epsilon = std::numeric_limits<float>::epsilon();
    constexpr auto side_a = tri.side_a_vector();
    constexpr auto side_b = tri.side_b_vector();
    const auto ray_dir = ray.direction_vector();
    const auto p = ray_dir.cross(side_b);
    const auto det = side_a.dot(p);

    if (std::abs(det) < k_epsilon)
    {
        EXPECT_EQ(hit, ray.end);
        return;
    }

    const auto inv_det = 1.0f / det;
    const auto tvec = ray.start - tri.m_vertex2;
    const auto q = tvec.cross(side_a);
    const auto t_hit = side_b.dot(q) * inv_det;

    if (t_hit <= k_epsilon || t_hit > 1.0f)
        EXPECT_EQ(hit, ray.end) << "t_hit=" << t_hit << " hit=" << hit.x << "," << hit.y << "," << hit.z;
    else
        EXPECT_NE(hit, ray.end) << "t_hit=" << t_hit << " hit=" << hit.x << "," << hit.y << "," << hit.z;
}

TEST(LineTracerMore, InfiniteLengthWithSmallTHitReturnsEnd)
{
    Triangle3 tri(Vector3<float>{0.f,0.f,0.f}, Vector3<float>{1.f,0.f,0.f}, Vector3<float>{0.f,1.f,0.f});
    constexpr Triangle3 tri2(Vector3<float>{0.f,0.f,-1e-8f}, Vector3<float>{1.f,0.f,-1e-8f}, Vector3<float>{0.f,1.f,-1e-8f});
    Ray ray; ray.start = {0.f,0.f,0.f}; ray.end = {0.f,0.f,1.f}; ray.infinite_length = true;
    // Create triangle slightly behind so t_hit <= eps
    tri = tri2;

    const auto hit = LineTracer<>::get_ray_hit_point(ray, tri);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerMore, SuccessfulHitReturnsPoint)
{
    constexpr Triangle3 tri(Vector3<float>{0.f,0.f,0.f}, Vector3<float>{1.f,0.f,0.f}, Vector3<float>{0.f,1.f,0.f});
    Ray ray; ray.start = {0.1f,0.1f,-1.f}; ray.end = {0.1f,0.1f,1.f};

    const auto hit = LineTracer<>::get_ray_hit_point(ray, tri);
    EXPECT_NE(hit, ray.end);
    // Hit should be on plane z=0 and near x=0.1,y=0.1
    EXPECT_NEAR(hit.z, 0.f, 1e-6f);
    EXPECT_NEAR(hit.x, 0.1f, 1e-3f);
    EXPECT_NEAR(hit.y, 0.1f, 1e-3f);
}
