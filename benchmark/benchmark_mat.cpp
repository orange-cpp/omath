//
// Created by Vlad on 9/17/2025.
//
#include <benchmark/benchmark.h>
#include <omath/omath.hpp>
using namespace omath;

void mat_float_multiplication_col_major(benchmark::State& state)
{
    using MatType = Mat<128, 128, float, MatStoreType::COLUMN_MAJOR>;
    MatType a;
    MatType b;
    a.set(3.f);
    b.set(7.f);

    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(a * b);
    }
}
void mat_float_multiplication_row_major(benchmark::State& state)
{
    using MatType = Mat<128, 128, float, MatStoreType::ROW_MAJOR>;
    MatType a;
    MatType b;
    a.set(3.f);
    b.set(7.f);

    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(a * b);
    }
}

void mat_double_multiplication_row_major(benchmark::State& state)
{
    using MatType = Mat<128, 128, double, MatStoreType::ROW_MAJOR>;
    MatType a;
    MatType b;
    a.set(3.f);
    b.set(7.f);

    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(a * b);
    }
}

void mat_double_multiplication_col_major(benchmark::State& state)
{
    using MatType = Mat<128, 128, double, MatStoreType::COLUMN_MAJOR>;
    MatType a;
    MatType b;
    a.set(3.f);
    b.set(7.f);

    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(a * b);
    }
}

// 4x4 is the only size the camera and the engine traits ever use, and it is far below the width of a single AVX2
// vector. Sizes either side of the 8 element vector width are measured too, so the point where the vectorised path
// starts paying off is visible rather than assumed.
#define MAT_SQUARE_MULTIPLICATION_BENCHMARK(size, scalar, order, suffix)                                               \
    void mat_##scalar##_multiplication_##size##x##size##_##suffix(benchmark::State& state)                             \
    {                                                                                                                  \
        using MatType = Mat<size, size, scalar, MatStoreType::order>;                                                  \
        MatType a;                                                                                                     \
        MatType b;                                                                                                     \
        a.set(static_cast<scalar>(3));                                                                                 \
        b.set(static_cast<scalar>(7));                                                                                 \
                                                                                                                       \
        for ([[maybe_unused]] const auto _ : state)                                                                    \
        {                                                                                                              \
            benchmark::DoNotOptimize(a);                                                                               \
            benchmark::DoNotOptimize(b);                                                                               \
            benchmark::DoNotOptimize(a * b);                                                                           \
        }                                                                                                              \
    }

MAT_SQUARE_MULTIPLICATION_BENCHMARK(4, float, ROW_MAJOR, row_major)
MAT_SQUARE_MULTIPLICATION_BENCHMARK(4, float, COLUMN_MAJOR, col_major)
MAT_SQUARE_MULTIPLICATION_BENCHMARK(4, double, ROW_MAJOR, row_major)
MAT_SQUARE_MULTIPLICATION_BENCHMARK(8, float, ROW_MAJOR, row_major)
MAT_SQUARE_MULTIPLICATION_BENCHMARK(16, float, ROW_MAJOR, row_major)

// The shape every world_to_screen call goes through: a 4x4 transform against a single point as a 4x1 column.
void mat_float_multiplication_4x4_by_4x1_row_major(benchmark::State& state)
{
    Mat<4, 4, float, MatStoreType::ROW_MAJOR> transform;
    transform.set(3.f);
    Vector3 point{128.f, -512.f, 64.f};

    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(transform);
        benchmark::DoNotOptimize(point);
        benchmark::DoNotOptimize(transform * mat_column_from_vector<float, MatStoreType::ROW_MAJOR>(point));
    }
}

// Cofactor expansion over 4x4: every screen_to_world call pays this, uncached.
void mat_float_4x4_inverted(benchmark::State& state)
{
    const Mat<4, 4, float, MatStoreType::ROW_MAJOR> mat{
            {0.9f, 0.1f, 0.05f, 12.f}, {0.1f, 1.1f, 0.2f, -4.f}, {0.f, 0.15f, 0.95f, 3.f}, {0.f, 0.f, 0.f, 1.f}};

    for ([[maybe_unused]] const auto _ : state)
    {
        benchmark::DoNotOptimize(mat);
        benchmark::DoNotOptimize(mat.inverted());
    }
}

BENCHMARK(mat_float_multiplication_col_major)->Iterations(5000);
BENCHMARK(mat_float_multiplication_row_major)->Iterations(5000);

BENCHMARK(mat_double_multiplication_col_major)->Iterations(5000);
BENCHMARK(mat_double_multiplication_row_major)->Iterations(5000);

BENCHMARK(mat_float_multiplication_4x4_row_major)->Iterations(5000000);
BENCHMARK(mat_float_multiplication_4x4_col_major)->Iterations(5000000);
BENCHMARK(mat_double_multiplication_4x4_row_major)->Iterations(5000000);
BENCHMARK(mat_float_multiplication_8x8_row_major)->Iterations(2000000);
BENCHMARK(mat_float_multiplication_16x16_row_major)->Iterations(500000);

BENCHMARK(mat_float_multiplication_4x4_by_4x1_row_major)->Iterations(5000000);
BENCHMARK(mat_float_4x4_inverted)->Iterations(2000000);
