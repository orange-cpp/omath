//
// Created by Vlad on 9/17/2025.
//
#include <benchmark/benchmark.h>

#include <omath/omath.hpp>
#include <chrono>
#include <print>

using namespace omath;

static void BM_MatMutiplication(benchmark::State& state)
{
    using mat_type = Mat<128, 128, float, MatStoreType::COLUMN_MAJOR>;
    mat_type a;
    mat_type b;
    a.set(3.f);
    b.set(7.f);


    for (auto _ : state)
    {
        std::ignore = a * b;
    }
}

BENCHMARK(BM_MatMutiplication);
BENCHMARK_MAIN();