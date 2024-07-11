cmake_minimum_required(VERSION 3.26)


project(omath)

set(CMAKE_CXX_STANDARD 26)

option(BUILD_TESTS "Build unit tests" ON)

add_library(omath STATIC source/Vector3.cpp)

add_subdirectory(source)
add_subdirectory(extlibs)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif ()

target_include_directories(omath PUBLIC include)