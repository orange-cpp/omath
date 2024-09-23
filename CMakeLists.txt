cmake_minimum_required(VERSION 3.26)


project(omath)

set(CMAKE_CXX_STANDARD 26)

option(OMATH_BUILD_TESTS "Build unit tests" ON)
option(OMATH_THREAT_WARNING_AS_ERROR "Set highest level of warnings and force compiler to threat them as errors" ON)
option(OMATH_BUILD_AS_SHARED_LIBRARY "Build Omath as .so\\.dll." OFF)

if (OMATH_BUILD_AS_SHARED_LIBRARY)
    add_library(omath SHARED source/Vector3.cpp)
else()
    add_library(omath STATIC source/Vector3.cpp)
endif()

add_subdirectory(source)
add_subdirectory(extlibs)

if(OMATH_BUILD_TESTS)
    add_subdirectory(tests)
endif ()

if (WIN32 AND OMATH_THREAT_WARNING_AS_ERROR)
    target_compile_options(omath PRIVATE /W4 /WX)
elseif(UNIX AND OMATH_THREAT_WARNING_AS_ERROR)
    target_compile_options(omath PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(omath PUBLIC include)