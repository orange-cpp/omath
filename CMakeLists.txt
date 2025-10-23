cmake_minimum_required(VERSION 3.26)

project(omath VERSION 3.9.2 LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(CheckCXXCompilerFlag)

if (MSVC)
    check_cxx_compiler_flag("/arch:AVX2" COMPILER_SUPPORTS_AVX2)
else ()
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
endif ()

option(OMATH_BUILD_TESTS "Build unit tests" ${PROJECT_IS_TOP_LEVEL})
option(OMATH_BUILD_BENCHMARK "Build benchmarks" ${PROJECT_IS_TOP_LEVEL})
option(OMATH_THREAT_WARNING_AS_ERROR "Set highest level of warnings and force compiler to treat them as errors" ON)
option(OMATH_BUILD_AS_SHARED_LIBRARY "Build Omath as .so or .dll" OFF)
option(OMATH_USE_AVX2 "Omath will use AVX2 to boost performance" ${COMPILER_SUPPORTS_AVX2})
option(OMATH_IMGUI_INTEGRATION "Omath will define method to convert omath types to imgui types" OFF)
option(OMATH_BUILD_EXAMPLES "Build example projects with you can learn & play" OFF)
option(OMATH_STATIC_MSVC_RUNTIME_LIBRARY "Force Omath to link static runtime" OFF)
option(OMATH_SUPRESS_SAFETY_CHECKS "Supress some safety checks in release build to improve general performance" ON)
option(OMATH_USE_UNITY_BUILD "Will enable unity build to speed up compilation" OFF)
option(OMATH_ENABLE_LEGACY "Will enable legacy classes that MUST be used ONLY for backward compatibility" OFF)

if (CMAKE_TOOLCHAIN_FILE)
    foreach (omath_feature IN LISTS VCPKG_MANIFEST_FEATURES)
        if (omath_feature STREQUAL "imgui")
            set(OMATH_IMGUI_INTEGRATION ON)
        elseif (omath_feature STREQUAL "avx2")
            set(OMATH_USE_AVX2 ${COMPILER_SUPPORTS_AVX2})
        endif ()
    endforeach ()
endif ()
if (OMATH_USE_AVX2 AND NOT COMPILER_SUPPORTS_AVX2)
    message(WARNING "OMATH_USE_AVX2 requested, but compiler/target does not support AVX2. Disabling.")
    set(OMATH_USE_AVX2 OFF CACHE BOOL "Omath will use AVX2 to boost performance" FORCE)
endif ()

if (${PROJECT_IS_TOP_LEVEL})
    message(STATUS "[${PROJECT_NAME}]: Building on ${CMAKE_HOST_SYSTEM_NAME}, compiler ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "[${PROJECT_NAME}]: Warnings as errors ${OMATH_THREAT_WARNING_AS_ERROR}")
    message(STATUS "[${PROJECT_NAME}]: Build unit tests ${OMATH_BUILD_TESTS}")
    message(STATUS "[${PROJECT_NAME}]: Build benchmark ${OMATH_BUILD_BENCHMARK}")
    message(STATUS "[${PROJECT_NAME}]: As dynamic library ${OMATH_BUILD_AS_SHARED_LIBRARY}")
    message(STATUS "[${PROJECT_NAME}]: Static C++ runtime ${OMATH_STATIC_MSVC_RUNTIME_LIBRARY}")
    message(STATUS "[${PROJECT_NAME}]: CMake unity build ${OMATH_USE_UNITY_BUILD}")
    message(STATUS "[${PROJECT_NAME}]: Example projects ${OMATH_BUILD_EXAMPLES}")
    message(STATUS "[${PROJECT_NAME}]: AVX2 feature status ${OMATH_USE_AVX2}")
    message(STATUS "[${PROJECT_NAME}]: ImGUI integration feature status ${OMATH_IMGUI_INTEGRATION}")
    message(STATUS "[${PROJECT_NAME}]: Legacy features support ${OMATH_ENABLE_LEGACY}")
endif ()

file(GLOB_RECURSE OMATH_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")
file(GLOB_RECURSE OMATH_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

if (OMATH_BUILD_AS_SHARED_LIBRARY)
    add_library(${PROJECT_NAME} SHARED ${OMATH_SOURCES} ${OMATH_HEADERS})
else ()
    add_library(${PROJECT_NAME} STATIC ${OMATH_SOURCES} ${OMATH_HEADERS})
endif ()


add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PUBLIC OMATH_VERSION="${PROJECT_VERSION}")

if (OMATH_IMGUI_INTEGRATION)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OMATH_IMGUI_INTEGRATION)

    # IMGUI is being linked as submodule
    if (TARGET imgui)
        target_link_libraries(${PROJECT_NAME} PUBLIC imgui)
        install(TARGETS imgui
                EXPORT omathTargets
                ARCHIVE DESTINATION lib
                LIBRARY DESTINATION lib
                RUNTIME DESTINATION bin)
    else ()
        # Assume that IMGUI linked via VCPKG.
        find_package(imgui CONFIG REQUIRED)
        target_link_libraries(${PROJECT_NAME} PUBLIC imgui::imgui)
    endif ()

endif ()

if (OMATH_USE_AVX2)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OMATH_USE_AVX2)
endif ()

if (OMATH_SUPRESS_SAFETY_CHECKS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OMATH_SUPRESS_SAFETY_CHECKS)
endif ()

if (OMATH_ENABLE_LEGACY)
    target_compile_options(${PROJECT_NAME} PUBLIC OMATH_ENABLE_LEGACY)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON)

if (OMATH_USE_UNITY_BUILD)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            UNITY_BUILD ON
            UNITY_BUILD_BATCH_SIZE 20)
endif ()

if (OMATH_STATIC_MSVC_RUNTIME_LIBRARY)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif ()

if (OMATH_USE_AVX2)
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PUBLIC /arch:AVX2)
    elseif (EMSCRIPTEN)
        target_compile_options(${PROJECT_NAME} PUBLIC -msimd128 -mavx2)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${PROJECT_NAME} PUBLIC -mfma -mavx2)
    endif ()
endif ()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

add_subdirectory(extlibs)


if (OMATH_BUILD_TESTS)
    add_subdirectory(tests)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OMATH_BUILD_TESTS)
endif ()

if (OMATH_BUILD_BENCHMARK)
    add_subdirectory(benchmark)
endif ()

if (OMATH_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND OMATH_THREAT_WARNING_AS_ERROR)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
elseif (OMATH_THREAT_WARNING_AS_ERROR)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ()

# Windows SDK redefine min/max via preprocessor and break std::min and std::max
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(${PROJECT_NAME} INTERFACE NOMINMAX)
endif ()

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>   # Use this path when building the project
        $<INSTALL_INTERFACE:include>                             # Use this path when the project is installed
)


# Installation rules

# Install the library
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION lib COMPONENT ${PROJECT_NAME}   # For static libraries
        LIBRARY DESTINATION lib COMPONENT ${PROJECT_NAME}   # For shared libraries
        RUNTIME DESTINATION bin COMPONENT ${PROJECT_NAME}   # For executables (on Windows)
)

# Install headers as part of omath_component
install(DIRECTORY include/ DESTINATION include COMPONENT ${PROJECT_NAME})

# Export omath target for CMake find_package support, also under omath_component
install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION lib/cmake/${PROJECT_NAME} COMPONENT ${PROJECT_NAME}
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
        INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

# Install the generated config files
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/omathConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/omathConfigVersion.cmake"
        DESTINATION lib/cmake/${PROJECT_NAME}
)
