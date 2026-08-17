//
// Created by Vlad on 9/18/2025.
//
#include <benchmark/benchmark.h>
#include <omath/omath.hpp>
using namespace omath;

using namespace omath::projectile_prediction;

constexpr float simulation_time_step = 1.f / 1000.f;
constexpr float hit_distance_tolerance = 5.f;

void source_engine_projectile_prediction(benchmark::State& state)
{
    constexpr Target<float> target{.m_origin = {100, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile<float> projectile = {.m_origin = {3, 2, 1}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};

    for ([[maybe_unused]] const auto _ : state)
        std::ignore = ProjPredEngineLegacy<>(400.f, simulation_time_step, 50.f, hit_distance_tolerance)
                              .maybe_calculate_aim_point(projectile, target);
}

BENCHMARK(source_engine_projectile_prediction)->Iterations(10'000);

// The scenarios below pin down the two halves of find_solution() separately. Every time step runs
// predict_target_position() plus the launch pitch solve; only steps that produce a pitch go on to
// is_projectile_reached_target(), which is the expensive half.
namespace
{
    using Engine = ProjPredEngineLegacy<>;

    // Solution found after a short scan - both halves run, but only for a handful of steps
    void projectile_prediction_short_range(benchmark::State& state)
    {
        constexpr Target<float> target{.m_origin = {100, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
        constexpr Projectile<float> projectile{
                .m_origin = {3, 2, 1}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};
        const Engine engine(400.f, simulation_time_step, 50.f, hit_distance_tolerance);

        for ([[maybe_unused]] const auto _ : state)
            benchmark::DoNotOptimize(engine.maybe_calculate_aim_point(projectile, target));
    }

    // Moving target far away - the scan runs for many more steps before it converges
    void projectile_prediction_long_range_moving(benchmark::State& state)
    {
        constexpr Target<float> target{.m_origin = {4000, 1500, 0}, .m_velocity = {-50, 20, 0}, .m_is_airborne = false};
        constexpr Projectile<float> projectile{
                .m_origin = {0, 0, 0}, .m_launch_speed = 3000.f, .m_gravity_scale = 1.0f};
        const Engine engine(800.f, simulation_time_step, 50.f, 10.f);

        for ([[maybe_unused]] const auto _ : state)
            benchmark::DoNotOptimize(engine.maybe_calculate_aim_point(projectile, target));
    }

    // Target out of range: the pitch solve always fails, so every step exits before
    // is_projectile_reached_target(). Isolates the cheap half over the full scan.
    void projectile_prediction_no_solution(benchmark::State& state)
    {
        constexpr Target<float> target{.m_origin = {100000, 0, 0}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
        constexpr Projectile<float> projectile{.m_origin = {0, 0, 0}, .m_launch_speed = 1.f, .m_gravity_scale = 1.f};
        const Engine engine(400.f, simulation_time_step, 5.f, hit_distance_tolerance);

        for ([[maybe_unused]] const auto _ : state)
            benchmark::DoNotOptimize(engine.maybe_calculate_aim_point(projectile, target));
    }

    // Same solve, but returning angles instead of an aim point
    void projectile_prediction_aim_angles(benchmark::State& state)
    {
        constexpr Target<float> target{.m_origin = {100, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
        constexpr Projectile<float> projectile{
                .m_origin = {3, 2, 1}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f};
        const Engine engine(400.f, simulation_time_step, 50.f, hit_distance_tolerance);

        for ([[maybe_unused]] const auto _ : state)
            benchmark::DoNotOptimize(engine.maybe_calculate_aim_angles(projectile, target));
    }
} // namespace

BENCHMARK(projectile_prediction_short_range)->Iterations(200'000);
BENCHMARK(projectile_prediction_long_range_moving)->Iterations(20'000);
BENCHMARK(projectile_prediction_no_solution)->Iterations(20'000);
BENCHMARK(projectile_prediction_aim_angles)->Iterations(200'000);