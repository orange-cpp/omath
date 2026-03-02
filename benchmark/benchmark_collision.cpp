//
// Created by Vlad on 3/2/2026.
//
#include <benchmark/benchmark.h>
#include <memory_resource>
#include <omath/collision/epa_algorithm.hpp>
#include <omath/collision/gjk_algorithm.hpp>
#include <omath/engines/source_engine/collider.hpp>
#include <omath/engines/source_engine/mesh.hpp>

using Mesh = omath::source_engine::Mesh;
using Collider = omath::source_engine::MeshCollider;
using Gjk = omath::collision::GjkAlgorithm<Collider>;
using Epa = omath::collision::Epa<Collider>;

namespace
{
    // Unit cube with half-extent 1 — 8 vertices in [-1,1]^3.
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
    const std::vector<omath::Vector3<std::uint32_t>> k_empty_vao{};
} // namespace

// ---------------------------------------------------------------------------
// GJK benchmarks
// ---------------------------------------------------------------------------

// Separated cubes — origin distance 2.1, no overlap.
// Exercises the early-exit path and the centroid-based initial direction.
static void BM_Gjk_Separated(benchmark::State& state)
{
    const Collider a{Mesh{k_cube_vbo, k_empty_vao}};

    Mesh mesh_b{k_cube_vbo, k_empty_vao};
    mesh_b.set_origin({0.f, 2.1f, 0.f});
    const Collider b{mesh_b};

    for ([[maybe_unused]] auto _ : state)
        benchmark::DoNotOptimize(Gjk::is_collide(a, b));
}

// Overlapping cubes — B offset by 0.5 along X, ~1.5 units penetration depth.
static void BM_Gjk_Overlapping(benchmark::State& state)
{
    const Collider a{Mesh{k_cube_vbo, k_empty_vao}};

    Mesh mesh_b{k_cube_vbo, k_empty_vao};
    mesh_b.set_origin({0.5f, 0.f, 0.f});
    const Collider b{mesh_b};

    for ([[maybe_unused]] auto _ : state)
        benchmark::DoNotOptimize(Gjk::is_collide(a, b));
}

// Identical cubes at the same origin — deep overlap / worst case for GJK.
static void BM_Gjk_SameOrigin(benchmark::State& state)
{
    const Collider a{Mesh{k_cube_vbo, k_empty_vao}};
    const Collider b{Mesh{k_cube_vbo, k_empty_vao}};

    for ([[maybe_unused]] auto _ : state)
        benchmark::DoNotOptimize(Gjk::is_collide(a, b));
}

// ---------------------------------------------------------------------------
// EPA benchmarks
// ---------------------------------------------------------------------------

// EPA with a pre-allocated monotonic buffer (reset each iteration).
// Isolates algorithmic cost from allocator overhead.
static void BM_Epa_MonotonicBuffer(benchmark::State& state)
{
    const Collider a{Mesh{k_cube_vbo, k_empty_vao}};

    Mesh mesh_b{k_cube_vbo, k_empty_vao};
    mesh_b.set_origin({0.5f, 0.f, 0.f});
    const Collider b{mesh_b};

    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(a, b);
    if (!hit)
        return; // shouldn't happen, but guard for safety

    constexpr Epa::Params params{.max_iterations = 64, .tolerance = 1e-4f};

    // Pre-allocate a 32 KiB stack buffer — enough for typical polytope growth.
    constexpr std::size_t k_buf_size = 32768;
    alignas(std::max_align_t) char buf[k_buf_size];
    std::pmr::monotonic_buffer_resource mr{buf, k_buf_size, std::pmr::null_memory_resource()};

    for ([[maybe_unused]] auto _ : state)
    {
        mr.release(); // reset the buffer without touching the upstream resource
        benchmark::DoNotOptimize(Epa::solve(a, b, simplex, params, mr));
    }
}

// EPA with the default (malloc-backed) memory resource.
// Shows total cost including allocator pressure.
static void BM_Epa_DefaultResource(benchmark::State& state)
{
    const Collider a{Mesh{k_cube_vbo, k_empty_vao}};

    Mesh mesh_b{k_cube_vbo, k_empty_vao};
    mesh_b.set_origin({0.5f, 0.f, 0.f});
    const Collider b{mesh_b};

    const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(a, b);
    if (!hit)
        return;

    constexpr Epa::Params params{.max_iterations = 64, .tolerance = 1e-4f};

    for ([[maybe_unused]] auto _ : state)
        benchmark::DoNotOptimize(Epa::solve(a, b, simplex, params));
}

// ---------------------------------------------------------------------------
// Combined GJK + EPA pipeline
// ---------------------------------------------------------------------------

// Full collision pipeline: GJK detects contact, EPA resolves penetration.
// This is the hot path in a physics engine tick.
static void BM_GjkEpa_Pipeline(benchmark::State& state)
{
    const Collider a{Mesh{k_cube_vbo, k_empty_vao}};

    Mesh mesh_b{k_cube_vbo, k_empty_vao};
    mesh_b.set_origin({0.5f, 0.f, 0.f});
    const Collider b{mesh_b};

    constexpr Epa::Params params{.max_iterations = 64, .tolerance = 1e-4f};

    constexpr std::size_t k_buf_size = 32768;
    alignas(std::max_align_t) char buf[k_buf_size];
    std::pmr::monotonic_buffer_resource mr{buf, k_buf_size, std::pmr::null_memory_resource()};

    for ([[maybe_unused]] auto _ : state)
    {
        mr.release();
        const auto [hit, simplex] = Gjk::is_collide_with_simplex_info(a, b);
        if (hit)
            benchmark::DoNotOptimize(Epa::solve(a, b, simplex, params, mr));
    }
}

BENCHMARK(BM_Gjk_Separated)->Iterations(100'000);
BENCHMARK(BM_Gjk_Overlapping)->Iterations(100'000);
BENCHMARK(BM_Gjk_SameOrigin)->Iterations(100'000);

BENCHMARK(BM_Epa_MonotonicBuffer)->Iterations(100'000);
BENCHMARK(BM_Epa_DefaultResource)->Iterations(100'000);

BENCHMARK(BM_GjkEpa_Pipeline)->Iterations(100'000);
