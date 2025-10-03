//
// Created by Vlad on 9/17/2025.
//
#include <benchmark/benchmark.h>

import omath.linear_algebra.mat;
using namespace omath;


void mat_float_multiplication_col_major(benchmark::State& state)
{
    using MatType = Mat<128, 128, float, MatStoreType::COLUMN_MAJOR>;
    MatType a;
    MatType b;
    a.set(3.f);
    b.set(7.f);


    for ([[maybe_unused]] const auto _ : state)
        std::ignore = a * b;
}
void mat_float_multiplication_row_major(benchmark::State& state)
{
    using MatType = Mat<128, 128, float, MatStoreType::ROW_MAJOR>;
    MatType a;
    MatType b;
    a.set(3.f);
    b.set(7.f);


    for ([[maybe_unused]] const auto _ : state)
        std::ignore = a * b;
}

void mat_double_multiplication_row_major(benchmark::State& state)
{
    using MatType = Mat<128, 128, double, MatStoreType::ROW_MAJOR>;
    MatType a;
    MatType b;
    a.set(3.f);
    b.set(7.f);


    for ([[maybe_unused]] const auto _ : state)
        std::ignore = a * b;
}

void mat_double_multiplication_col_major(benchmark::State& state)
{
    using MatType = Mat<128, 128, double, MatStoreType::COLUMN_MAJOR>;
    MatType a;
    MatType b;
    a.set(3.f);
    b.set(7.f);


    for ([[maybe_unused]] const auto _ : state)
        std::ignore = a * b;
}

BENCHMARK(mat_float_multiplication_col_major)->Iterations(5000);
BENCHMARK(mat_float_multiplication_row_major)->Iterations(5000);

BENCHMARK(mat_double_multiplication_col_major)->Iterations(5000);
BENCHMARK(mat_double_multiplication_row_major)->Iterations(5000);