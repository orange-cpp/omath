//
// Created by Vlad on 9/17/2025.
//
#include <benchmark/benchmark.h>

#include <omath/omath.hpp>
#include <chrono>
#include <print>

using namespace omath;


void mat_multiplication(benchmark::State& state)
{
    using MatType = Mat<128, 128, float, MatStoreType::COLUMN_MAJOR>;
    MatType a;
    MatType b;
    a.set(3.f);
    b.set(7.f);


    for (auto _ : state)
    {
        std::ignore = a * b;
    }
}

BENCHMARK(mat_multiplication);
BENCHMARK_MAIN();