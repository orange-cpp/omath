//
// Created by Vlad on 9/18/2025.
//
#include <benchmark/benchmark.h>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>

namespace
{
    using Engine = omath::projectile_prediction::ProjPredEngineLegacy<>;
    using Projectile = omath::projectile_prediction::Projectile<float>;
    using Target = omath::projectile_prediction::Target<float>;

    struct PredictionScenario
    {
        Projectile projectile;
        Target target;
        float gravity;
        float simulation_time_step;
        float maximum_simulation_time;
        float distance_tolerance;
        bool expects_solution;
    };

    void run_prediction_benchmark(benchmark::State& state, const PredictionScenario& scenario)
    {
        const Engine engine(scenario.gravity, scenario.simulation_time_step, scenario.maximum_simulation_time,
                            scenario.distance_tolerance);
        auto projectile = scenario.projectile;
        auto target = scenario.target;

        if (engine.maybe_calculate_aim_point(projectile, target).has_value() != scenario.expects_solution)
        {
            state.SkipWithError("Projectile benchmark scenario returned an unexpected result");
            return;
        }

        for ([[maybe_unused]] const auto _ : state)
        {
            benchmark::DoNotOptimize(projectile);
            benchmark::DoNotOptimize(target);
            auto result = engine.maybe_calculate_aim_point(projectile, target);
            benchmark::DoNotOptimize(result);
        }
    }

    void projectile_prediction_near_static_hit(benchmark::State& state)
    {
        constexpr PredictionScenario scenario{
                .projectile = {.m_origin = {3.f, 2.f, 1.f}, .m_launch_speed = 5000.f, .m_gravity_scale = 0.4f},
                .target = {.m_origin = {100.f, 0.f, 90.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false},
                .gravity = 400.f,
                .simulation_time_step = 1.f / 1000.f,
                .maximum_simulation_time = 50.f,
                .distance_tolerance = 5.f,
                .expects_solution = true,
        };
        run_prediction_benchmark(state, scenario);
    }

    void projectile_prediction_moving_hit(benchmark::State& state)
    {
        constexpr PredictionScenario scenario{
                .projectile = {.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 3000.f, .m_gravity_scale = 1.f},
                .target = {.m_origin = {500.f, 100.f, 0.f}, .m_velocity = {-50.f, 20.f, 0.f}, .m_is_airborne = false},
                .gravity = 800.f,
                .simulation_time_step = 1.f / 500.f,
                .maximum_simulation_time = 30.f,
                .distance_tolerance = 10.f,
                .expects_solution = true,
        };
        run_prediction_benchmark(state, scenario);
    }

    void projectile_prediction_unreachable_full_scan(benchmark::State& state)
    {
        constexpr PredictionScenario scenario{
                .projectile = {.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 1.f, .m_gravity_scale = 1.f},
                .target = {.m_origin = {100'000.f, 0.f, 0.f}, .m_velocity = {0.f, 0.f, 0.f}, .m_is_airborne = false},
                .gravity = 9.81f,
                .simulation_time_step = 1.f / 1000.f,
                .maximum_simulation_time = 2.f,
                .distance_tolerance = 5.f,
                .expects_solution = false,
        };
        run_prediction_benchmark(state, scenario);
    }

    void projectile_prediction_receding_full_scan(benchmark::State& state)
    {
        constexpr PredictionScenario scenario{
                .projectile = {.m_origin = {0.f, 0.f, 0.f}, .m_launch_speed = 100.f, .m_gravity_scale = 0.f},
                .target = {.m_origin = {100.f, 0.f, 0.f}, .m_velocity = {200.f, 0.f, 0.f}, .m_is_airborne = false},
                .gravity = 9.81f,
                .simulation_time_step = 1.f / 1000.f,
                .maximum_simulation_time = 2.f,
                .distance_tolerance = 0.01f,
                .expects_solution = false,
        };
        run_prediction_benchmark(state, scenario);
    }
} // namespace

BENCHMARK(projectile_prediction_near_static_hit);
BENCHMARK(projectile_prediction_moving_hit);
BENCHMARK(projectile_prediction_unreachable_full_scan);
BENCHMARK(projectile_prediction_receding_full_scan);
