project(omath_benchmark)


file(GLOB_RECURSE OMATH_BENCHMARK_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(${PROJECT_NAME} ${OMATH_BENCHMARK_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON)

if (TARGET benchmark::benchmark) # Benchmark is being linked as submodule
    target_link_libraries(${PROJECT_NAME} PRIVATE benchmark::benchmark omath)
else()
    find_package(benchmark CONFIG REQUIRED) # Benchmark is being linked as vcpkg package
    target_link_libraries(${PROJECT_NAME} PRIVATE benchmark::benchmark omath)
endif ()

if(OMATH_ENABLE_VALGRIND)
    omath_setup_valgrind(${PROJECT_NAME})
endif()
