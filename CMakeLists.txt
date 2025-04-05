cmake_minimum_required(VERSION 3.26)

project(omath VERSION 1.0.1 LANGUAGES CXX)

include(CMakePackageConfigHelpers)


option(OMATH_BUILD_TESTS "Build unit tests" ON)
option(OMATH_THREAT_WARNING_AS_ERROR "Set highest level of warnings and force compiler to treat them as errors" ON)
option(OMATH_BUILD_AS_SHARED_LIBRARY "Build Omath as .so or .dll" OFF)
option(OMATH_USE_AVX2 "Omath will use AVX2 to boost performance" ON)
option(OMATH_IMGUI_INTEGRATION "Omath will define method to convert omath types to imgui types" OFF)
option(OMATH_BUILD_EXAMPLES "Build example projects with you can learn & play" OFF)
option(OMATH_STATIC_MSVC_RUNTIME_LIBRARY  "Force Omath to link static runtime" OFF)

if (OMATH_BUILD_AS_SHARED_LIBRARY)
    add_library(omath SHARED source/matrix.cpp)
else()
    add_library(omath STATIC source/matrix.cpp
            source/matrix.cpp)
endif()
message(STATUS "Building on ${CMAKE_HOST_SYSTEM_NAME}")
add_library(omath::omath ALIAS omath)

if (OMATH_IMGUI_INTEGRATION)
    target_compile_definitions(omath PUBLIC OMATH_IMGUI_INTEGRATION)

    # IMGUI is being linked as submodule
    if(TARGET imgui)
        target_link_libraries(omath PUBLIC imgui)
        install(TARGETS imgui
                EXPORT omathTargets
                ARCHIVE DESTINATION lib
                LIBRARY DESTINATION lib
                RUNTIME DESTINATION bin)
    else()
        # Assume that IMGUI linked via VCPKG.
        find_package(imgui CONFIG REQUIRED)
        target_link_libraries(omath PUBLIC imgui::imgui)
    endif()

endif()

if (OMATH_USE_AVX2)
    target_compile_definitions(omath PUBLIC OMATH_USE_AVX2)
endif()

set_target_properties(omath PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        UNITY_BUILD ON
        UNITY_BUILD_BATCH_SIZE 20
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON)

if (OMATH_STATIC_MSVC_RUNTIME_LIBRARY)
    set_target_properties(omath PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(omath PRIVATE -mavx2 -mfma)
endif()

target_compile_features(omath PUBLIC cxx_std_23)


add_subdirectory(source)

if(OMATH_BUILD_TESTS)
    add_subdirectory(extlibs)
    add_subdirectory(tests)
endif()

if (OMATH_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND OMATH_THREAT_WARNING_AS_ERROR)
    target_compile_options(omath PRIVATE /W4 /WX)
elseif(OMATH_THREAT_WARNING_AS_ERROR)
    target_compile_options(omath PRIVATE -Wall -Wextra -Wpedantic -Werror)
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
