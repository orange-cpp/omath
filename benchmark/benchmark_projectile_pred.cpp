//
// Created by Vlad on 9/18/2025.
//
#include <benchmark/benchmark.h>
import omath.projectile_prediction.engine_legacy;
using namespace omath;

using namespace omath::projectile_prediction;

constexpr float simulation_time_step = 1.f / 1000.f;
constexpr float hit_distance_tolerance = 5.f;

void source_engine_projectile_prediction(benchmark::State& state)
{
    constexpr Target target{.m_origin = {100, 0, 90}, .m_velocity = {0, 0, 0}, .m_is_airborne = false};
    constexpr Projectile projectile = {.m_origin = {3, 2, 1}, .m_launch_speed = 5000, .m_gravity_scale = 0.4};

    for ([[maybe_unused]] const auto _: state)
        std::ignore = ProjPredEngineLegacy(400, simulation_time_step, 50, hit_distance_tolerance)
                              .maybe_calculate_aim_point(projectile, target);
}

BENCHMARK(source_engine_projectile_prediction)->Iterations(10'000);