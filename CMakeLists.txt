cmake_minimum_required(VERSION 3.26)

project(omath VERSION 1.0.0)



set(CMAKE_CXX_STANDARD 26)

option(OMATH_BUILD_TESTS "Build unit tests" ON)
option(OMATH_THREAT_WARNING_AS_ERROR "Set highest level of warnings and force compiler to treat them as errors" ON)
option(OMATH_BUILD_AS_SHARED_LIBRARY "Build Omath as .so or .dll" OFF)

if (OMATH_BUILD_AS_SHARED_LIBRARY)
    add_library(omath SHARED source/Vector3.cpp)
else()
    add_library(omath STATIC source/Vector3.cpp)
endif()

add_subdirectory(source)
add_subdirectory(extlibs)

if(OMATH_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if (WIN32 AND OMATH_THREAT_WARNING_AS_ERROR)
    target_compile_options(omath PRIVATE /W4 /WX)
elseif(UNIX AND OMATH_THREAT_WARNING_AS_ERROR)
    target_compile_options(omath PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(omath
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>   # Use this path when building the project
        $<INSTALL_INTERFACE:include>                             # Use this path when the project is installed
)


# Installation rules

# Install the library
install(TARGETS omath
        EXPORT omathTargets
        ARCHIVE DESTINATION lib COMPONENT omath   # For static libraries
        LIBRARY DESTINATION lib COMPONENT omath   # For shared libraries
        RUNTIME DESTINATION bin COMPONENT omath   # For executables (on Windows)
)

# Install headers as part of omath_component
install(DIRECTORY include/ DESTINATION include COMPONENT omath)

# Export omath target for CMake find_package support, also under omath_component
install(EXPORT omathTargets
        FILE omathTargets.cmake
        NAMESPACE omath::
        DESTINATION lib/cmake/omath COMPONENT omath
)


# Generate the omathConfigVersion.cmake file
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/omathConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

# Generate the omathConfig.cmake file from the template (which is in the cmake/ folder)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/omathConfig.cmake.in"  # Path to the .in file
        "${CMAKE_CURRENT_BINARY_DIR}/omathConfig.cmake"           # Output path for the generated file
        INSTALL_DESTINATION lib/cmake/omath
)

# Install the generated config files
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/omathConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/omathConfigVersion.cmake"
        DESTINATION lib/cmake/omath
)