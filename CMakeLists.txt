cmake_minimum_required(VERSION 3.26)
project(uml)

set(CMAKE_CXX_STANDARD 26)
option(BUILD_TESTS "Build test programs" ON)
add_library(uml STATIC source/Vector3.cpp)

add_subdirectory(source)
add_subdirectory(extlibs)
add_subdirectory(tests)

target_include_directories(uml PUBLIC include)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()