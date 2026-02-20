# cmake/run.examples.cmake

# Get the project root directory (assuming this script is in cmake/ subdirectory)
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

# Default to Debug if CMAKE_BUILD_TYPE is not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
else()
    message(STATUS "CMAKE_BUILD_TYPE is set to: ${CMAKE_BUILD_TYPE}")
endif()

# Define the directory where executables are located
# Based on CMakeLists.txt: "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
set(EXAMPLES_BIN_DIR "${PROJECT_ROOT}/out/${CMAKE_BUILD_TYPE}")

if(NOT EXISTS "${EXAMPLES_BIN_DIR}")
    message(
        FATAL_ERROR
            "Examples binary directory not found: ${EXAMPLES_BIN_DIR}. Please build the project first."
        )
endif()

message(STATUS "Looking for benchmark executables in: ${EXAMPLES_BIN_DIR}")

# Find all files starting with "omath_benchmark"
file(GLOB EXAMPLE_FILES "${EXAMPLES_BIN_DIR}/omath_benchmark*")

foreach(EXAMPLE_PATH ${EXAMPLE_FILES})
    # Skip directories
    if(IS_DIRECTORY "${EXAMPLE_PATH}")
        continue()
    endif()

    get_filename_component(FILENAME "${EXAMPLE_PATH}" NAME)
    get_filename_component(EXTENSION "${EXAMPLE_PATH}" EXT)

    # Filter out potential debug symbols or non-executable artifacts
    if(EXTENSION STREQUAL ".pdb" OR EXTENSION STREQUAL ".ilk" OR EXTENSION STREQUAL ".obj")
        continue()
    endif()

    # On Windows, we expect .exe
    if(MSVC AND NOT EXTENSION STREQUAL ".exe")
        continue()
    endif()

    # On Linux/macOS, check permissions or just try to run it.

    message(STATUS "-------------------------------------------------")
    message(STATUS "Running benchmark: ${FILENAME}")
    message(STATUS "-------------------------------------------------")

    execute_process(COMMAND "${EXAMPLE_PATH}" WORKING_DIRECTORY "${PROJECT_ROOT}"
                    RESULT_VARIABLE EXIT_CODE)

    if(NOT EXIT_CODE EQUAL 0)
        message(WARNING "Benchmark ${FILENAME} exited with error code: ${EXIT_CODE}")
    else()
        message(STATUS "Benchmark ${FILENAME} completed successfully.")
    endif()

endforeach()
