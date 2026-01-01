enable_testing()

project(unit_tests)

include(GoogleTest)

file(GLOB_RECURSE UNIT_TESTS_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(${PROJECT_NAME} ${UNIT_TESTS_SOURCES})

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

if(MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++exp)
endif()

if(OMATH_ENABLE_COVERAGE)
    include(${CMAKE_SOURCE_DIR}/cmake/Coverage.cmake)
    omath_setup_coverage(${PROJECT_NAME})
endif()

if(OMATH_ENABLE_VALGRIND)
    omath_setup_valgrind(${PROJECT_NAME})
endif()

# Skip test discovery for Android/iOS builds or when cross-compiling - binaries cannot run on host
if (NOT (ANDROID OR IOS OR EMSCRIPTEN))
    gtest_discover_tests(${PROJECT_NAME})
endif()
