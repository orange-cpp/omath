//
// Created by Vlad on 9/17/2025.
//
#include <gtest/gtest.h>
#include <omath/omath.hpp>
#include <chrono>
#include <print>

using namespace omath;
TEST(MatPerformanceTest, Mutl)
{
    using mat_type = Mat<128, 128, float, MatStoreType::COLUMN_MAJOR>;
    mat_type a;
    mat_type b;
    a.set(3.f);
    b.set(7.f);
    constexpr int iters = 1000;
    float acum_time = 0.f;
    mat_type c;
    for (std::size_t i = 0 ; i < iters; i++)
    {
        const auto start = std::chrono::high_resolution_clock::now();
        c = a * b;
        const auto end = std::chrono::high_resolution_clock::now();

        const auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        acum_time += static_cast<float>(time);
    }
    std::print("Elapsed: {}, \n\n\n", acum_time / iters, c);
}