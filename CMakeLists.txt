cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 26)
project(uml)

add_library(uml source/Vector3.cpp)
add_subdirectory(source)

target_include_directories(uml PUBLIC include)