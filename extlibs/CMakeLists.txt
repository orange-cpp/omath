
if (OMATH_BUILD_TESTS)
    add_subdirectory(googletest)
endif ()

if (OMATH_BUILD_BENCHMARK)
    set(BENCHMARK_ENABLE_TESTING OFF)
    add_subdirectory(benchmark)
endif ()