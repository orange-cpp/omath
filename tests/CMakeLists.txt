enable_testing()

project(unit_tests)

include(GoogleTest)

include(${CMAKE_SOURCE_DIR}/cmake/Coverage.cmake)

file(GLOB_RECURSE UNIT_TESTS_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(${PROJECT_NAME} ${UNIT_TESTS_SOURCES})

# Add project include directory so tests can include headers using
# paths like "omath/..."). Avoid adding the project root which can
# accidentally expose repository files (for example a top-level
# VERSION file) as headers and collide with system includes.
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/source/coverage)

# Compile specific TU without inlining to help coverage attribute header lines
if(OMATH_ENABLE_COVERAGE AND CMAKE_HOST_LINUX AND OMATH_BUILD_TESTS)
    get_property(project_sources DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY FILES)
    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/general/coverage_instantiations.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/general/coverage_utility_instantiations.cpp
        PROPERTIES COMPILE_OPTIONS "-fno-inline")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON)

if (TARGET gtest) # GTest is being linked as submodule
    target_link_libraries(${PROJECT_NAME} PRIVATE gtest gtest_main omath::omath)
else() # GTest is being linked as vcpkg package
    find_package(GTest CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE GTest::gtest GTest::gtest_main omath::omath)
endif()

# If building with Clang and coverage enabled, remove any GCC-style
# Delegate test coverage wiring and test discovery to cmake/Coverage.cmake
if(OMATH_ENABLE_COVERAGE AND CMAKE_HOST_LINUX AND OMATH_BUILD_TESTS)
    omath_setup_coverage_for_test(${PROJECT_NAME})
endif()

# Skip test discovery for Android/iOS builds or when cross-compiling - binaries cannot run on host
if (NOT (ANDROID OR IOS OR EMSCRIPTEN))
    gtest_discover_tests(${PROJECT_NAME})
endif()
