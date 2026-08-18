//
// Created by Vlad on 8/18/2026.
//
#include "omath/collision/line_tracer.hpp"
#include "omath/engines/source_engine/mesh.hpp"
#include <gtest/gtest.h>

using Vec3 = omath::Vector3<float>;
using Ray = omath::collision::Ray<>;
using LineTracer = omath::collision::LineTracer<>;
using Mesh = omath::source_engine::Mesh;

namespace
{
    Ray make_ray(const Vec3 start, const Vec3 end, const bool infinite = false)
    {
        Ray r;
        r.start = start;
        r.end = end;
        r.infinite_length = infinite;
        return r;
    }

    // Unit cube spanning [-1, 1]^3 as 12 triangles, so a ray through it crosses two faces and the tracer has to pick
    // the nearer one.
    Mesh make_cube()
    {
        std::vector<omath::primitives::Vertex<>> vbo = {
                {{-1.f, -1.f, -1.f}, {}, {}}, {{1.f, -1.f, -1.f}, {}, {}}, {{1.f, 1.f, -1.f}, {}, {}},
                {{-1.f, 1.f, -1.f}, {}, {}},  {{-1.f, -1.f, 1.f}, {}, {}}, {{1.f, -1.f, 1.f}, {}, {}},
                {{1.f, 1.f, 1.f}, {}, {}},    {{-1.f, 1.f, 1.f}, {}, {}},
        };

        std::vector<omath::Vector3<std::uint32_t>> ebo = {
                {0, 1, 2}, {0, 2, 3}, // -Z
                {4, 6, 5}, {4, 7, 6}, // +Z
                {0, 5, 1}, {0, 4, 5}, // -Y
                {3, 2, 6}, {3, 6, 7}, // +Y
                {0, 3, 7}, {0, 7, 4}, // -X
                {1, 5, 6}, {1, 6, 2}, // +X
        };

        return Mesh{std::move(vbo), std::move(ebo)};
    }

    // The world-space algorithm the local-space tracer replaced, kept here as an oracle: transform every face into
    // world space, intersect, keep the nearest. Any disagreement with LineTracer means the local-space shortcut is
    // not the identity it claims to be.
    Vec3 reference_trace(const Ray& ray, const Mesh& mesh)
    {
        auto best = ray.end;

        for (auto it = mesh.m_element_buffer_object.cbegin(); it != mesh.m_element_buffer_object.cend(); ++it)
        {
            const auto hit = LineTracer::get_ray_hit_point(ray, mesh.make_face_in_world_space(it));

            if (hit.distance_to(ray.start) < best.distance_to(ray.start))
                best = hit;
        }

        return best;
    }
} // namespace

TEST(LineTracerMeshTests, HitsNearestFace)
{
    const auto mesh = make_cube();
    const auto ray = make_ray({0.f, 0.f, -5.f}, {0.f, 0.f, 5.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, mesh);

    EXPECT_NE(hit, ray.end);
    // -Z face, not the +Z one four units further along
    EXPECT_NEAR(hit.z, -1.f, 1e-4f);
    EXPECT_NEAR(hit.x, 0.f, 1e-4f);
    EXPECT_NEAR(hit.y, 0.f, 1e-4f);
}

TEST(LineTracerMeshTests, MissReturnsRayEnd)
{
    const auto mesh = make_cube();
    const auto ray = make_ray({5.f, 5.f, -5.f}, {5.f, 5.f, 5.f});

    EXPECT_EQ(LineTracer::get_ray_hit_point(ray, mesh), ray.end);
}

// A ray that stops before reaching the mesh is a miss, not a hit past its own end point.
TEST(LineTracerMeshTests, RayTooShortReturnsRayEnd)
{
    const auto mesh = make_cube();
    const auto ray = make_ray({0.f, 0.f, -5.f}, {0.f, 0.f, -3.f});

    EXPECT_EQ(LineTracer::get_ray_hit_point(ray, mesh), ray.end);
}

TEST(LineTracerMeshTests, TranslatedMeshShiftsHit)
{
    auto mesh = make_cube();
    mesh.set_origin({0.f, 0.f, 10.f});

    const auto ray = make_ray({0.f, 0.f, -5.f}, {0.f, 0.f, 20.f});
    const auto hit = LineTracer::get_ray_hit_point(ray, mesh);

    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.z, 9.f, 1e-4f);
}

// Non-uniform scale is the case the local-space shortcut has to earn: distances are not preserved by the transform,
// only the ray parameter t is.
TEST(LineTracerMeshTests, NonUniformScaleHitsScaledSurface)
{
    auto mesh = make_cube();
    mesh.set_scale({1.f, 1.f, 4.f});

    const auto ray = make_ray({0.f, 0.f, -20.f}, {0.f, 0.f, 20.f});
    const auto hit = LineTracer::get_ray_hit_point(ray, mesh);

    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.z, -4.f, 1e-3f);
}

TEST(LineTracerMeshTests, DegenerateScaleReturnsRayEnd)
{
    auto mesh = make_cube();
    mesh.set_scale({0.f, 0.f, 0.f});

    const auto ray = make_ray({0.f, 0.f, -5.f}, {0.f, 0.f, 5.f});

    EXPECT_EQ(LineTracer::get_ray_hit_point(ray, mesh), ray.end);
}

// local_bounds is the broadphase callers are meant to hoist, so it has to be tight and independent of the transform.
TEST(LineTracerMeshTests, LocalBoundsCoverVertexBuffer)
{
    auto mesh = make_cube();
    mesh.set_origin({100.f, 100.f, 100.f});
    mesh.set_scale({5.f, 5.f, 5.f});

    const auto bounds = mesh.local_bounds();

    EXPECT_EQ(bounds.min, Vec3(-1.f, -1.f, -1.f));
    EXPECT_EQ(bounds.max, Vec3(1.f, 1.f, 1.f));
}

// A slab test against the hoisted bounds must not reject a ray the full trace would have hit.
TEST(LineTracerMeshTests, LocalBoundsDoNotRejectRealHits)
{
    const auto mesh = make_cube();
    const auto bounds = mesh.local_bounds();

    for (int i = 0; i < 32; ++i)
    {
        const auto angle = 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / 32.f;
        const Vec3 offset{std::cos(angle) * 6.f, static_cast<float>(i % 5) - 2.f, std::sin(angle) * 6.f};

        const auto ray = make_ray(offset, -offset);

        if (LineTracer::get_ray_hit_point(ray, mesh) != ray.end)
            EXPECT_NE(LineTracer::get_ray_hit_point(ray, bounds), ray.end) << "ray " << i;
    }
}

// The load-bearing test: agreement with the world-space reference across a transform that combines translation,
// rotation and non-uniform scale, over rays hitting from every direction.
TEST(LineTracerMeshTests, MatchesWorldSpaceReference)
{
    auto mesh = make_cube();
    mesh.set_origin({3.f, -2.f, 7.f});
    mesh.set_scale({2.f, 0.5f, 1.5f});
    mesh.set_rotation({omath::source_engine::PitchAngle::from_degrees(25.f),
                       omath::source_engine::YawAngle::from_degrees(40.f),
                       omath::source_engine::RollAngle::from_degrees(15.f)});

    const auto center = mesh.get_origin();

    for (int i = 0; i < 64; ++i)
    {
        const auto angle = 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / 64.f;
        const Vec3 offset{std::cos(angle) * 12.f, static_cast<float>(i % 7) - 3.f, std::sin(angle) * 12.f};

        const auto ray = make_ray(center + offset, center - offset);

        const auto actual = LineTracer::get_ray_hit_point(ray, mesh);
        const auto expected = reference_trace(ray, mesh);

        EXPECT_NEAR(actual.x, expected.x, 1e-3f) << "ray " << i;
        EXPECT_NEAR(actual.y, expected.y, 1e-3f) << "ray " << i;
        EXPECT_NEAR(actual.z, expected.z, 1e-3f) << "ray " << i;
    }
}

// Same agreement check for rays that miss, where both paths must produce the ray.end sentinel exactly.
TEST(LineTracerMeshTests, MatchesWorldSpaceReferenceOnMisses)
{
    auto mesh = make_cube();
    mesh.set_origin({3.f, -2.f, 7.f});
    mesh.set_rotation({omath::source_engine::PitchAngle::from_degrees(25.f),
                       omath::source_engine::YawAngle::from_degrees(40.f),
                       omath::source_engine::RollAngle::from_degrees(15.f)});

    for (int i = 0; i < 32; ++i)
    {
        const auto angle = 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / 32.f;
        const Vec3 start = mesh.get_origin() + Vec3{std::cos(angle) * 40.f, 30.f, std::sin(angle) * 40.f};

        const auto ray = make_ray(start, start + Vec3{0.f, 5.f, 0.f});

        EXPECT_EQ(LineTracer::get_ray_hit_point(ray, mesh), ray.end) << "ray " << i;
        EXPECT_EQ(reference_trace(ray, mesh), ray.end) << "ray " << i;
    }
}
