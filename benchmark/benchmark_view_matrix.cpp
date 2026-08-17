//
// Created by Vlad on 8/17/2026.
//
#include <benchmark/benchmark.h>
#include <omath/omath.hpp>
// omath.hpp pulls in every other engine, but not this one
#include <omath/engines/cry_engine/formulas.hpp>
using namespace omath;

// Every engine builds its view matrix the same way - a rotation from the view angles, three basis vectors pulled out of
// it, then mat_camera_view(). The engines differ only in axis convention, storage order and scalar type, so they are
// all measured here to catch a regression that hits just one of them.
#define VIEW_MATRIX_BENCHMARK(engine, scalar)                                                                          \
    void view_matrix_##engine(benchmark::State& state)                                                                 \
    {                                                                                                                  \
        engine::ViewAngles angles{engine::PitchAngle::from_degrees(23), engine::YawAngle::from_degrees(114),           \
                                  engine::RollAngle::from_degrees(7)};                                                 \
        Vector3<scalar> origin{128, -512, 64};                                                                         \
                                                                                                                       \
        for ([[maybe_unused]] const auto _ : state)                                                                    \
        {                                                                                                              \
            benchmark::DoNotOptimize(angles);                                                                          \
            benchmark::DoNotOptimize(origin);                                                                          \
            benchmark::DoNotOptimize(engine::calc_view_matrix(angles, origin));                                        \
        }                                                                                                              \
    }

VIEW_MATRIX_BENCHMARK(source_engine, float)
VIEW_MATRIX_BENCHMARK(iw_engine, float)
VIEW_MATRIX_BENCHMARK(cry_engine, float)
VIEW_MATRIX_BENCHMARK(frostbite_engine, float)
VIEW_MATRIX_BENCHMARK(rage_engine, float)
VIEW_MATRIX_BENCHMARK(unity_engine, float)
VIEW_MATRIX_BENCHMARK(opengl_engine, float)
VIEW_MATRIX_BENCHMARK(unreal_engine, double)

// The two halves of a view matrix, measured on their own so a regression can be attributed to one of them
void view_matrix_rotation_only(benchmark::State& state)
{
    source_engine::ViewAngles angles{source_engine::PitchAngle::from_degrees(23),
                                     source_engine::YawAngle::from_degrees(114),
                                     source_engine::RollAngle::from_degrees(7)};

    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(angles);
        benchmark::DoNotOptimize(source_engine::rotation_matrix(angles));
    }
}

void view_matrix_camera_view_only(benchmark::State& state)
{
    Vector3<float> forward{0.91f, 0.39f, -0.12f};
    Vector3<float> right{-0.39f, 0.92f, 0.f};
    Vector3<float> up{0.11f, 0.05f, 0.99f};
    Vector3<float> origin{128, -512, 64};

    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(forward);
        benchmark::DoNotOptimize(right);
        benchmark::DoNotOptimize(up);
        benchmark::DoNotOptimize(origin);
        benchmark::DoNotOptimize(mat_camera_view<float, MatStoreType::ROW_MAJOR>(forward, right, up, origin));
    }
}

BENCHMARK(view_matrix_source_engine)->Iterations(2000000);
BENCHMARK(view_matrix_iw_engine)->Iterations(2000000);
BENCHMARK(view_matrix_cry_engine)->Iterations(2000000);
BENCHMARK(view_matrix_frostbite_engine)->Iterations(2000000);
BENCHMARK(view_matrix_rage_engine)->Iterations(2000000);
BENCHMARK(view_matrix_unity_engine)->Iterations(2000000);
BENCHMARK(view_matrix_opengl_engine)->Iterations(2000000);
BENCHMARK(view_matrix_unreal_engine)->Iterations(2000000);

BENCHMARK(view_matrix_rotation_only)->Iterations(2000000);
BENCHMARK(view_matrix_camera_view_only)->Iterations(2000000);
