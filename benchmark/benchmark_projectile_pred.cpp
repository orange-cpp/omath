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

    for ([[maybe_unused]] const auto _: state)
        std::ignore = ProjPredEngineLegacy<>(400.f, simulation_time_step, 50.f, hit_distance_tolerance)
                              .maybe_calculate_aim_point(projectile, target);
}

BENCHMARK(source_engine_projectile_prediction)->Iterations(10'000);