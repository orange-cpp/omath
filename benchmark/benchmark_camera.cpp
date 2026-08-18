//
// Created by Vlad on 8/18/2026.
//
#include <benchmark/benchmark.h>
#include <omath/omath.hpp>
#include <vector>

using namespace omath;

namespace
{
    // Deterministic so a before/after comparison sees the exact same inputs. std::mt19937 would do, but the
    // distributions on top of it are not specified to produce identical output across standard libraries, and these
    // numbers end up baked into results compared across machines.
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
    constexpr std::size_t k_point_count = 1024;
    constexpr std::size_t k_point_mask = k_point_count - 1;

    // An arbitrary world point for the camera to face. Everything else is built from the camera's own basis vectors, so
    // these benchmarks do not care which axis a given engine calls forward.
    constexpr Vector3<float> k_look_target = {512.f, 96.f, 48.f};

    enum class CloudKind
    {
        IN_FRONT, // inside the frustum - a visible entity
        BEHIND, // mirrored through the camera origin, so w <= 0 - the early-out path
        MIXED, // half and half, which is what a real per-frame entity list looks like
    };

    template<class CameraType>
    [[nodiscard]]
    CameraType make_camera()
    {
        using Vec = std::decay_t<decltype(std::declval<const CameraType&>().get_origin())>;
        using Scalar = typename Vec::ContainedType;

        CameraType camera{Vec{0, 0, 0},
                          {},
                          projection::ViewPort{1920.f, 1080.f},
                          projection::FieldOfView::from_degrees(90.f),
                          static_cast<Scalar>(0.1),
                          static_cast<Scalar>(10000)};
        camera.look_at({static_cast<Scalar>(k_look_target.x), static_cast<Scalar>(k_look_target.y),
                        static_cast<Scalar>(k_look_target.z)});
        return camera;
    }

    template<class CameraType>
    [[nodiscard]]
    auto make_point_cloud(const CameraType& camera, const CloudKind kind)
    {
        using Vec = std::decay_t<decltype(camera.get_origin())>;
        using Scalar = typename Vec::ContainedType;

        // Offsets are applied along the camera's own axes, so the cloud lands in the frustum for every engine
        // regardless of its axis convention.
        const auto forward = camera.get_abs_forward();
        const auto right = camera.get_abs_right();
        const auto up = camera.get_abs_up();
        const auto origin = camera.get_origin();

        Rng rng;
        std::vector<Vec> points;
        points.reserve(k_point_count);

        for (std::size_t i = 0; i < k_point_count; ++i)
        {
            const auto depth = static_cast<Scalar>(rng.next(128.f, 2048.f));
            // tan(21.8 deg) - comfortably inside a 90 degree horizontal FOV on every engine
            const auto spread = depth * static_cast<Scalar>(0.4);
            const auto lateral = static_cast<Scalar>(rng.next(-1.f, 1.f)) * spread;
            const auto vertical = static_cast<Scalar>(rng.next(-1.f, 1.f)) * spread;

            const auto in_front = kind == CloudKind::IN_FRONT || (kind == CloudKind::MIXED && i % 2 == 0);
            const auto signed_depth = in_front ? depth : -depth;

            points.push_back(origin + forward * signed_depth + right * lateral + up * vertical);
        }

        return points;
    }

    // Reported once per benchmark rather than measured, so we can tell at a glance whether a cloud is actually
    // exercising the path it is named after instead of silently landing on the error branch.
    template<class CameraType, class Points>
    void report_projection_rate(benchmark::State& state, const CameraType& camera, const Points& points)
    {
        std::size_t projected = 0;
        for (const auto& point : points)
            if (camera.world_to_screen(point).has_value())
                ++projected;

        state.counters["on_screen_pct"] =
                static_cast<double>(projected) * 100.0 / static_cast<double>(points.size());
    }
} // namespace

// ── world_to_screen ──────────────────────────────────────────────────────────
// The library's hottest entry point: an overlay calls it once per entity vertex, every frame. Measured on three
// engines to cover both storage orders and both scalar types, since the projection code is shared by all of them.
#define WORLD_TO_SCREEN_BENCHMARK(engine, kind, suffix)                                                                \
    void camera_world_to_screen_##suffix##_##engine(benchmark::State& state)                                           \
    {                                                                                                                  \
        const auto camera = make_camera<engine::Camera>();                                                             \
        const auto points = make_point_cloud(camera, CloudKind::kind);                                                 \
        report_projection_rate(state, camera, points);                                                                 \
                                                                                                                       \
        std::size_t index = 0;                                                                                         \
        for ([[maybe_unused]] const auto _ : state)                                                                    \
        {                                                                                                              \
            benchmark::DoNotOptimize(camera);                                                                          \
            benchmark::DoNotOptimize(camera.world_to_screen(points[index]));                                           \
            index = (index + 1) & k_point_mask;                                                                        \
        }                                                                                                              \
    }

WORLD_TO_SCREEN_BENCHMARK(source_engine, IN_FRONT, visible)
WORLD_TO_SCREEN_BENCHMARK(opengl_engine, IN_FRONT, visible)
WORLD_TO_SCREEN_BENCHMARK(unreal_engine, IN_FRONT, visible)

// Every one of these bails on w <= 0. Today that check happens after the full 4x4 transform has already run.
WORLD_TO_SCREEN_BENCHMARK(source_engine, BEHIND, behind)
WORLD_TO_SCREEN_BENCHMARK(opengl_engine, BEHIND, behind)
WORLD_TO_SCREEN_BENCHMARK(unreal_engine, BEHIND, behind)

WORLD_TO_SCREEN_BENCHMARK(source_engine, MIXED, mixed)

void camera_world_to_view_coordinates(benchmark::State& state)
{
    const auto camera = make_camera<source_engine::Camera>();
    const auto points = make_point_cloud(camera, CloudKind::IN_FRONT);

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(camera);
        benchmark::DoNotOptimize(camera.world_to_view_coordinates(points[index]));
        index = (index + 1) & k_point_mask;
    }
}

// Mirrors what EntityOverlay::from_aabb does per entity - project all 8 box corners plus the center - without
// dragging in a renderer. This is the number that actually shows up in a frame budget.
void camera_project_entity_box(benchmark::State& state)
{
    const auto camera = make_camera<source_engine::Camera>();
    const auto points = make_point_cloud(camera, CloudKind::MIXED);

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        const auto& center = points[index];
        const primitives::Aabb<float> box{center - Vector3<float>{16.f, 16.f, 36.f},
                                          center + Vector3<float>{16.f, 16.f, 36.f}};

        benchmark::DoNotOptimize(camera);
        for (const auto& vertex : box.vertices())
            benchmark::DoNotOptimize(camera.world_to_screen_unclipped(vertex));
        benchmark::DoNotOptimize(camera.world_to_screen_unclipped(box.center()));

        index = (index + 1) & k_point_mask;
    }
}

// ── screen_to_world ──────────────────────────────────────────────────────────
// Inverts the view-projection matrix on every single call, with nothing cached between calls.
void camera_screen_to_world(benchmark::State& state)
{
    const auto camera = make_camera<source_engine::Camera>();

    Rng rng;
    std::vector<Vector3<float>> screen_points;
    screen_points.reserve(k_point_count);
    for (std::size_t i = 0; i < k_point_count; ++i)
        screen_points.push_back({rng.next(0.f, 1920.f), rng.next(0.f, 1080.f), rng.next(0.f, 1.f)});

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(camera);
        benchmark::DoNotOptimize(camera.screen_to_world(screen_points[index]));
        index = (index + 1) & k_point_mask;
    }
}

// The same call with the camera moved between every query, so the cached inverse never hits. A camera that moves once
// per frame and is queried once per frame lands here, not on the benchmark above.
void camera_screen_to_world_moving(benchmark::State& state)
{
    auto camera = make_camera<source_engine::Camera>();

    Rng rng;
    std::vector<Vector3<float>> screen_points;
    screen_points.reserve(k_point_count);
    for (std::size_t i = 0; i < k_point_count; ++i)
        screen_points.push_back({rng.next(0.f, 1920.f), rng.next(0.f, 1080.f), rng.next(0.f, 1.f)});

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        camera.set_origin({static_cast<float>(index), 0.f, 64.f});
        benchmark::DoNotOptimize(camera);
        benchmark::DoNotOptimize(camera.screen_to_world(screen_points[index]));
        index = (index + 1) & k_point_mask;
    }
}

// ── frustum culling ──────────────────────────────────────────────────────────
// All three of these re-derive the six frustum planes from the cached view-projection matrix on every call.
void camera_is_aabb_culled_by_frustum(benchmark::State& state)
{
    const auto camera = make_camera<source_engine::Camera>();
    const auto centers = make_point_cloud(camera, CloudKind::MIXED);

    std::vector<primitives::Aabb<float>> boxes;
    boxes.reserve(k_point_count);
    for (const auto& center : centers)
        boxes.push_back({center - Vector3<float>{16.f, 16.f, 36.f}, center + Vector3<float>{16.f, 16.f, 36.f}});

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(camera);
        benchmark::DoNotOptimize(camera.is_aabb_culled_by_frustum(boxes[index]));
        index = (index + 1) & k_point_mask;
    }
}

void camera_is_obb_culled_by_frustum(benchmark::State& state)
{
    const auto camera = make_camera<source_engine::Camera>();
    const auto centers = make_point_cloud(camera, CloudKind::MIXED);

    std::vector<primitives::Obb<float>> boxes;
    boxes.reserve(k_point_count);
    for (const auto& center : centers)
        boxes.push_back({center,
                         Vector3<float>{1.f, 0.f, 0.f},
                         Vector3<float>{0.f, 1.f, 0.f},
                         Vector3<float>{0.f, 0.f, 1.f},
                         Vector3<float>{16.f, 16.f, 36.f}});

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(camera);
        benchmark::DoNotOptimize(camera.is_obb_culled_by_frustum(boxes[index]));
        index = (index + 1) & k_point_mask;
    }
}

void camera_is_triangle_culled_by_frustum(benchmark::State& state)
{
    const auto camera = make_camera<source_engine::Camera>();
    const auto centers = make_point_cloud(camera, CloudKind::MIXED);

    std::vector<Triangle<Vector3<float>>> triangles;
    triangles.reserve(k_point_count);
    for (const auto& center : centers)
        triangles.emplace_back(center, center + Vector3<float>{32.f, 0.f, 0.f},
                               center + Vector3<float>{0.f, 32.f, 24.f});

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(camera);
        benchmark::DoNotOptimize(camera.is_culled_by_frustum(triangles[index]));
        index = (index + 1) & k_point_mask;
    }
}

// ── cache invalidation ───────────────────────────────────────────────────────
// A camera that moves every frame pays a full view matrix rebuild (six sin/cos) plus a 4x4 by 4x4 multiply before the
// first projection lands. Contrast with camera_world_to_screen_visible_source_engine, which is the cache-hit case.
void camera_set_origin_then_project(benchmark::State& state)
{
    auto camera = make_camera<source_engine::Camera>();
    const auto points = make_point_cloud(camera, CloudKind::IN_FRONT);

    std::size_t index = 0;
    for ([[maybe_unused]] const auto _ : state)
    {
        camera.set_origin({static_cast<float>(index), 0.f, 64.f});
        benchmark::DoNotOptimize(camera);
        benchmark::DoNotOptimize(camera.world_to_screen(points[index]));
        index = (index + 1) & k_point_mask;
    }
}

BENCHMARK(camera_world_to_screen_visible_source_engine)->Iterations(2000000);
BENCHMARK(camera_world_to_screen_visible_opengl_engine)->Iterations(2000000);
BENCHMARK(camera_world_to_screen_visible_unreal_engine)->Iterations(2000000);

BENCHMARK(camera_world_to_screen_behind_source_engine)->Iterations(2000000);
BENCHMARK(camera_world_to_screen_behind_opengl_engine)->Iterations(2000000);
BENCHMARK(camera_world_to_screen_behind_unreal_engine)->Iterations(2000000);

BENCHMARK(camera_world_to_screen_mixed_source_engine)->Iterations(2000000);
BENCHMARK(camera_world_to_view_coordinates)->Iterations(2000000);
BENCHMARK(camera_project_entity_box)->Iterations(500000);

BENCHMARK(camera_screen_to_world)->Iterations(500000);
BENCHMARK(camera_screen_to_world_moving)->Iterations(500000);

BENCHMARK(camera_is_aabb_culled_by_frustum)->Iterations(2000000);
BENCHMARK(camera_is_obb_culled_by_frustum)->Iterations(2000000);
BENCHMARK(camera_is_triangle_culled_by_frustum)->Iterations(2000000);

BENCHMARK(camera_set_origin_then_project)->Iterations(1000000);
