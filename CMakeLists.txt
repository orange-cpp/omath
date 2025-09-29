cmake_minimum_required(VERSION 4.1)


project(omath VERSION 3.5.0 LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
option(OMATH_BUILD_TESTS "Build unit tests" ${PROJECT_IS_TOP_LEVEL})
option(OMATH_THREAT_WARNING_AS_ERROR "Set highest level of warnings and force compiler to treat them as errors" ON)
option(OMATH_BUILD_AS_SHARED_LIBRARY "Build Omath as .so or .dll" OFF)
option(OMATH_USE_AVX2 "Omath will use AVX2 to boost performance" ON)
option(OMATH_IMGUI_INTEGRATION "Omath will define method to convert omath types to imgui types" OFF)
option(OMATH_BUILD_EXAMPLES "Build example projects with you can learn & play" OFF)
option(OMATH_STATIC_MSVC_RUNTIME_LIBRARY "Force Omath to link static runtime" ON)
option(OMATH_SUPRESS_SAFETY_CHECKS "Supress some safety checks in release build to improve general performance" ON)
option(OMATH_USE_UNITY_BUILD "Will enable unity build to speed up compilation" OFF)
option(OMATH_ENABLE_LEGACY "Will enable legacy classes that MUST be used ONLY for backward compatibility" OFF)

message(STATUS "[${PROJECT_NAME}]: Building on ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "[${PROJECT_NAME}]: Warnings as errors ${OMATH_THREAT_WARNING_AS_ERROR}")
message(STATUS "[${PROJECT_NAME}]: Build unit tests ${OMATH_BUILD_TESTS}")
message(STATUS "[${PROJECT_NAME}]: As dynamic library ${OMATH_BUILD_AS_SHARED_LIBRARY}")
message(STATUS "[${PROJECT_NAME}]: Static C++ runtime ${OMATH_STATIC_MSVC_RUNTIME_LIBRARY}")
message(STATUS "[${PROJECT_NAME}]: CMake unity build ${OMATH_USE_UNITY_BUILD}")
message(STATUS "[${PROJECT_NAME}]: Example projects ${OMATH_BUILD_EXAMPLES}")
message(STATUS "[${PROJECT_NAME}]: AVX2 feature status ${OMATH_USE_AVX2}")
message(STATUS "[${PROJECT_NAME}]: ImGUI integration feature status ${OMATH_IMGUI_INTEGRATION}")
message(STATUS "[${PROJECT_NAME}]: Legacy features support ${OMATH_ENABLE_LEGACY}")
message(STATUS "[${PROJECT_NAME}]: MSVC Runtime ${CMAKE_MSVC_RUNTIME_LIBRARY}")
file(GLOB_RECURSE OMATH_MODULES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/modules/*.ixx")

if (OMATH_BUILD_AS_SHARED_LIBRARY)
    add_library(${PROJECT_NAME} SHARED)
else ()
    add_library(${PROJECT_NAME} STATIC)
endif ()

target_sources(${PROJECT_NAME}
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/modules
        FILES ${OMATH_MODULES}
)
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
            MSVC_RUNTIME_LIBRARY "MultiThreaded")
elseif(DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}")
endif()


target_compile_options(omath PUBLIC
        $<$<CXX_COMPILER_ID:Clang>:-mavx2 -mfma>
        $<$<CXX_COMPILER_ID:GNU>:-mavx2 -mfma>
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)


if (OMATH_BUILD_TESTS)
    add_subdirectory(extlibs)
    add_subdirectory(tests)
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

# Installation rules

# Install the library
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        FILE_SET cxx_modules DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/modules
        CXX_MODULES_BMI DESTINATION ${CMAKE_INSTALL_LIBDIR}/cxx/bmi/$<CONFIG>  # <— config-split
)

# Use a RELATIVE directory for module metadata beside the config files
install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION lib/cmake/${PROJECT_NAME}
        CXX_MODULES_DIRECTORY cxx-modules
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
