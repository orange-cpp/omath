//
// Created by Vlad on 8/18/2026.
//
#include <benchmark/benchmark.h>
#include <cmath>
#include <numbers>
#include <omath/collision/line_tracer.hpp>
#include <omath/engines/source_engine/mesh.hpp>
#include <vector>

using namespace omath;

namespace
{
    using Ray = collision::Ray<Vector3<float>>;
    using Tracer = collision::LineTracer<Ray>;
    using TriangleF = Triangle<Vector3<float>>;
    using AabbF = primitives::Aabb<float>;
    using ObbF = primitives::Obb<float>;
    using Mesh = source_engine::Mesh;

    // Same generator as benchmark_camera.cpp, for the same reason: the distributions layered on std::mt19937 are not
    // specified to produce identical output across standard libraries, and these numbers get baked into results that
    // are compared across machines.
    class Rng final
    {
    public:
        [[nodiscard]]
        float next(const float min, const float max) noexcept
        {
            m_state = m_state * 6364136223846793005ULL + 1442695040888963407ULL;
            const auto unit = static_cast<float>(m_state >> 40) / static_cast<float>(1 << 24);
            return min + (max - min) * unit;
        }

    private:
        uint64_t m_state = 0x853c49e6748fea9bULL;
    };

    // Power of two so the wrap in the measured loop is a mask instead of a division
    constexpr std::size_t k_case_count = 1024;
    constexpr std::size_t k_case_mask = k_case_count - 1;

    [[nodiscard]]
    bool is_hit(const Vector3<float>& result, const Ray& ray) noexcept
    {
        // The library signals "no intersection" by handing back ray.end verbatim, so an exact compare is the check.
        return !(result == ray.end);
    }

    // Reported rather than measured, so a glance at the output confirms a case is exercising the path it is named
    // after instead of silently landing on the opposite branch.
    template<class Shape>
    void report_hit_rate(benchmark::State& state, const std::vector<Ray>& rays, const Shape& shape)
    {
        std::size_t hits = 0;
        for (const auto& ray : rays)
            if (is_hit(Tracer::get_ray_hit_point(ray, shape), ray))
                ++hits;

        state.counters["hit_pct"] = static_cast<double>(hits) * 100.0 / static_cast<double>(rays.size());
    }

    // ── geometry ─────────────────────────────────────────────────────────────
    // A UV sphere of radius 1 centred on the origin. rings == sectors == n gives 2n^2 triangles, which is the knob
    // the mesh benchmarks sweep to expose the per-face cost.
    [[nodiscard]]
    Mesh make_sphere_mesh(const std::uint32_t n)
    {
        std::vector<primitives::Vertex<>> vbo;
        vbo.reserve((n + 1) * (n + 1));

        for (std::uint32_t ring = 0; ring <= n; ++ring)
        {
            const auto phi = std::numbers::pi_v<float> * static_cast<float>(ring) / static_cast<float>(n);
            for (std::uint32_t sector = 0; sector <= n; ++sector)
            {
                const auto theta =
                        2.f * std::numbers::pi_v<float> * static_cast<float>(sector) / static_cast<float>(n);
                const Vector3<float> position{std::sin(phi) * std::cos(theta), std::cos(phi),
                                              std::sin(phi) * std::sin(theta)};
                vbo.push_back({position, position, {}});
            }
        }

        std::vector<Vector3<std::uint32_t>> ebo;
        ebo.reserve(n * n * 2);

        for (std::uint32_t ring = 0; ring < n; ++ring)
        {
            for (std::uint32_t sector = 0; sector < n; ++sector)
            {
                const auto top_left = ring * (n + 1) + sector;
                const auto bottom_left = top_left + n + 1;

                ebo.push_back({top_left, bottom_left, top_left + 1});
                ebo.push_back({top_left + 1, bottom_left, bottom_left + 1});
            }
        }

        return Mesh{std::move(vbo), std::move(ebo)};
    }

    enum class RayKind
    {
        HIT, // aimed at the target, terminates past it
        MISS, // aimed well clear of the target
        GRAZE, // passes just outside the target's silhouette - no broadphase rejects this cheaply
    };

    // Rays fired inward from a shell of radius 8 at a unit-ish target sitting on the origin.
    [[nodiscard]]
    std::vector<Ray> make_rays(const RayKind kind)
    {
        Rng rng;
        std::vector<Ray> rays;
        rays.reserve(k_case_count);

        for (std::size_t i = 0; i < k_case_count; ++i)
        {
            const auto phi = rng.next(0.f, std::numbers::pi_v<float>);
            const auto theta = rng.next(0.f, 2.f * std::numbers::pi_v<float>);
            const Vector3<float> dir{std::sin(phi) * std::cos(theta), std::cos(phi), std::sin(phi) * std::sin(theta)};

            const auto start = dir * 8.f;

            // An offset perpendicular to the ray shifts the aim point off the origin without changing the start.
            const auto perpendicular = dir.cross({0.f, 1.f, 0.f}).normalized();
            const auto aim_offset = kind == RayKind::HIT ? 0.f : (kind == RayKind::GRAZE ? 1.9f : 6.f);

            rays.push_back({start, perpendicular * aim_offset - dir * 8.f, false});
        }

        return rays;
    }
} // namespace

// ── triangle (Möller–Trumbore) ───────────────────────────────────────────────
// The kernel every mesh trace runs per face. The miss case is the one that matters: in a mesh of any size almost
// every face is rejected, so the rejection path is what the per-face cost actually is.
void line_tracer_triangle_hit(benchmark::State& state)
{
    const auto rays = make_rays(RayKind::HIT);
    // Large enough to catch every ray in the set, centred on the origin the rays are aimed at.
    const TriangleF triangle{{-4.f, 0.f, -4.f}, {4.f, 0.f, -4.f}, {0.f, 0.f, 4.f}};
    report_hit_rate(state, rays, triangle);

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(Tracer::get_ray_hit_point(rays[index], triangle));
        index = (index + 1) & k_case_mask;
    }
}

void line_tracer_triangle_miss(benchmark::State& state)
{
    const auto rays = make_rays(RayKind::MISS);
    const TriangleF triangle{{-0.5f, 0.f, -0.5f}, {0.5f, 0.f, -0.5f}, {0.f, 0.f, 0.5f}};
    report_hit_rate(state, rays, triangle);

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(Tracer::get_ray_hit_point(rays[index], triangle));
        index = (index + 1) & k_case_mask;
    }
}

// Rejected on the determinant before any division happens - the cheapest exit the kernel has.
void line_tracer_triangle_parallel(benchmark::State& state)
{
    std::vector<Ray> rays;
    rays.reserve(k_case_count);

    Rng rng;
    for (std::size_t i = 0; i < k_case_count; ++i)
    {
        const auto height = rng.next(1.f, 4.f);
        rays.push_back({{-8.f, height, 0.f}, {16.f, 0.f, 0.f}, false});
    }

    const TriangleF triangle{{-1.f, 0.f, -1.f}, {1.f, 0.f, -1.f}, {0.f, 0.f, 1.f}};
    report_hit_rate(state, rays, triangle);

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(Tracer::get_ray_hit_point(rays[index], triangle));
        index = (index + 1) & k_case_mask;
    }
}

// ── AABB / OBB slab tests ────────────────────────────────────────────────────
// Three divisions and three branches per test today. These are the numbers a cached reciprocal direction or a
// branchless slab formulation would have to beat.
void line_tracer_aabb_hit(benchmark::State& state)
{
    const auto rays = make_rays(RayKind::HIT);
    const AabbF aabb{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    report_hit_rate(state, rays, aabb);

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(Tracer::get_ray_hit_point(rays[index], aabb));
        index = (index + 1) & k_case_mask;
    }
}

void line_tracer_aabb_miss(benchmark::State& state)
{
    const auto rays = make_rays(RayKind::MISS);
    const AabbF aabb{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    report_hit_rate(state, rays, aabb);

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(Tracer::get_ray_hit_point(rays[index], aabb));
        index = (index + 1) & k_case_mask;
    }
}

// Same slab test plus six dot products to fold the ray into the box's frame.
void line_tracer_obb_hit(benchmark::State& state)
{
    const auto rays = make_rays(RayKind::HIT);
    const ObbF obb{{0.f, 0.f, 0.f},
                   {0.7071068f, 0.7071068f, 0.f},
                   {-0.7071068f, 0.7071068f, 0.f},
                   {0.f, 0.f, 1.f},
                   {1.f, 1.f, 1.f}};
    report_hit_rate(state, rays, obb);

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(Tracer::get_ray_hit_point(rays[index], obb));
        index = (index + 1) & k_case_mask;
    }
}

void line_tracer_obb_miss(benchmark::State& state)
{
    const auto rays = make_rays(RayKind::MISS);
    const ObbF obb{{0.f, 0.f, 0.f},
                   {0.7071068f, 0.7071068f, 0.f},
                   {-0.7071068f, 0.7071068f, 0.f},
                   {0.f, 0.f, 1.f},
                   {1.f, 1.f, 1.f}};
    report_hit_rate(state, rays, obb);

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(Tracer::get_ray_hit_point(rays[index], obb));
        index = (index + 1) & k_case_mask;
    }
}

// ── mesh ─────────────────────────────────────────────────────────────────────
// The headline number. Every case below scans all 2n^2 faces with no bounding-volume rejection and no early exit,
// transforming three vertices through the full 4x4 world matrix per face. Sweeping n shows the cost is linear in
// face count, and comparing hit against miss shows it is the same line either way.
void line_tracer_mesh_hit(benchmark::State& state)
{
    const auto mesh = make_sphere_mesh(static_cast<std::uint32_t>(state.range(0)));
    const auto rays = make_rays(RayKind::HIT);
    report_hit_rate(state, rays, mesh);
    state.counters["faces"] = static_cast<double>(mesh.m_element_buffer_object.size());

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(Tracer::get_ray_hit_point(rays[index], mesh));
        index = (index + 1) & k_case_mask;
    }
}

// A clean miss costs exactly as much as a hit, because the loop has nothing to reject against.
void line_tracer_mesh_miss(benchmark::State& state)
{
    const auto mesh = make_sphere_mesh(static_cast<std::uint32_t>(state.range(0)));
    const auto rays = make_rays(RayKind::MISS);
    report_hit_rate(state, rays, mesh);
    state.counters["faces"] = static_cast<double>(mesh.m_element_buffer_object.size());

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(Tracer::get_ray_hit_point(rays[index], mesh));
        index = (index + 1) & k_case_mask;
    }
}

// The case a bounding volume cannot reject: the ray passes just outside the silhouette, so every face still has to
// be tested. This is the honest worst case and the floor any broadphase has to live with.
void line_tracer_mesh_graze(benchmark::State& state)
{
    const auto mesh = make_sphere_mesh(static_cast<std::uint32_t>(state.range(0)));
    const auto rays = make_rays(RayKind::GRAZE);
    report_hit_rate(state, rays, mesh);
    state.counters["faces"] = static_cast<double>(mesh.m_element_buffer_object.size());

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(Tracer::get_ray_hit_point(rays[index], mesh));
        index = (index + 1) & k_case_mask;
    }
}

// A mesh that moves between queries. set_origin drops the cached world matrix, so the first face of the next trace
// pays the rebuild - which is what an animated hitbox traced once per frame actually does.
void line_tracer_mesh_moving(benchmark::State& state)
{
    auto mesh = make_sphere_mesh(16);
    const auto rays = make_rays(RayKind::HIT);
    state.counters["faces"] = static_cast<double>(mesh.m_element_buffer_object.size());

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        mesh.set_origin({static_cast<float>(index) * 0.001f, 0.f, 0.f});
        benchmark::DoNotOptimize(Tracer::get_ray_hit_point(rays[index], mesh));
        index = (index + 1) & k_case_mask;
    }
}

BENCHMARK(line_tracer_triangle_hit)->Iterations(2000000);
BENCHMARK(line_tracer_triangle_miss)->Iterations(2000000);
BENCHMARK(line_tracer_triangle_parallel)->Iterations(2000000);

BENCHMARK(line_tracer_aabb_hit)->Iterations(2000000);
BENCHMARK(line_tracer_aabb_miss)->Iterations(2000000);
BENCHMARK(line_tracer_obb_hit)->Iterations(2000000);
BENCHMARK(line_tracer_obb_miss)->Iterations(2000000);

// n = 8/16/32 -> 128/512/2048 faces
BENCHMARK(line_tracer_mesh_hit)->Arg(8)->Arg(16)->Arg(32)->Iterations(20000);
BENCHMARK(line_tracer_mesh_miss)->Arg(8)->Arg(16)->Arg(32)->Iterations(20000);
BENCHMARK(line_tracer_mesh_graze)->Arg(8)->Arg(16)->Arg(32)->Iterations(20000);
BENCHMARK(line_tracer_mesh_moving)->Iterations(20000);
