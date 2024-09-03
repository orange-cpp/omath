cmake_minimum_required(VERSION 3.26)


project(omath)

set(CMAKE_CXX_STANDARD 26)

option(BUILD_TESTS "Build unit tests" ON)
option(THREAT_WARNING_AS_ERROR "Set highest level of warnings and force compiler to threat them as errors" ON)
add_library(omath STATIC source/Vector3.cpp)

add_subdirectory(source)
add_subdirectory(extlibs)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif ()

if (WIN32 AND THREAT_WARNING_AS_ERROR)
    target_compile_options(omath PRIVATE /W4 /WX)
elseif(UNIX AND THREAT_WARNING_AS_ERROR)
    target_compile_options(omath PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(omath PUBLIC include)