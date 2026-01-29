# cmake/Format.cmake

# Find cmake-format executable
find_program(CMAKE_FORMAT_EXECUTABLE NAMES cmake-format)

if(NOT CMAKE_FORMAT_EXECUTABLE)
    message(FATAL_ERROR "cmake-format not found. Please install it first.")
endif()

# Get the project root directory (assuming this script is in cmake/
# subdirectory)
get_filename_component(PROJECT_ROOT "../${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)

# Iterate over all files in the project root
file(GLOB_RECURSE ALL_FILES "${PROJECT_ROOT}/*")

foreach(FILE ${ALL_FILES})
    # Basic ignores for common directories to avoid formatting external/build
    # files Note: We check for substrings in the full path
    if("${FILE}" MATCHES "/\\.git/"
       OR "${FILE}" MATCHES "/build/"
       OR "${FILE}" MATCHES "/cmake-build/"
       OR "${FILE}" MATCHES "/\\.pixi/"
       OR "${FILE}" MATCHES "/vcpkg_installed/")
        continue()
    endif()

    get_filename_component(FILENAME "${FILE}" NAME)

    # Check if file ends with .cmake or matches exactly to CMakeLists.txt
    if("${FILENAME}" STREQUAL "CMakeLists.txt" OR "${FILENAME}" MATCHES "\\.cmake$")
        message(STATUS "Formatting ${FILE}")
        execute_process(COMMAND ${CMAKE_FORMAT_EXECUTABLE} --config-files
                                "${PROJECT_ROOT}/.cmake-format" -i "${FILE}")
    endif()
endforeach()
