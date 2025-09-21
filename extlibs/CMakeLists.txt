
if (OMATH_BUILD_TESTS)
    add_subdirectory(googletest)
endif ()

if (OMATH_BUILD_BENCHMARK)
add_subdirectory(benchmark)
endif()