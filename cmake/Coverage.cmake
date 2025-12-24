# cmake/Coverage.cmake
include_guard(GLOBAL)

function(omath_setup_coverage TARGET_NAME)
    if(ANDROID OR IOS OR EMSCRIPTEN)
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        # Apply to ALL configs when coverage is enabled (not just Debug)
        target_compile_options(${TARGET_NAME} PRIVATE
            -fprofile-instr-generate
            -fcoverage-mapping
            -g
            -O0
        )
        target_link_options(${TARGET_NAME} PRIVATE
            -fprofile-instr-generate
            -fcoverage-mapping
        )

    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${TARGET_NAME} PRIVATE
            --coverage
            -g
            -O0
        )
        target_link_options(${TARGET_NAME} PRIVATE
            --coverage
        )

    elseif(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
            /Zi
            /Od
            /Ob0
        )
        target_link_options(${TARGET_NAME} PRIVATE
            /DEBUG:FULL
            /INCREMENTAL:NO
        )
    endif()

    # Create coverage target only once
    if(TARGET coverage)
        return()
    endif()

    if(MSVC OR MINGW)
        # Windows: OpenCppCoverage
        find_program(OPENCPPCOVERAGE_EXECUTABLE 
            NAMES OpenCppCoverage OpenCppCoverage.exe
            PATHS
                "$ENV{ProgramFiles}/OpenCppCoverage"
                "$ENV{ProgramW6432}/OpenCppCoverage"
                "C:/Program Files/OpenCppCoverage"
            DOC "Path to OpenCppCoverage executable"
        )

        if(NOT OPENCPPCOVERAGE_EXECUTABLE)
            message(WARNING "OpenCppCoverage not found. Install with: choco install opencppcoverage")
            set(OPENCPPCOVERAGE_EXECUTABLE "C:/Program Files/OpenCppCoverage/OpenCppCoverage.exe")
        else()
            message(STATUS "Found OpenCppCoverage: ${OPENCPPCOVERAGE_EXECUTABLE}")
        endif()

        file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}" COVERAGE_ROOT_PATH)
        file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/coverage" COVERAGE_OUTPUT_PATH)
        file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/coverage.xml" COVERAGE_XML_PATH)
        file(TO_NATIVE_PATH "${OPENCPPCOVERAGE_EXECUTABLE}" OPENCPPCOVERAGE_NATIVE)

        add_custom_target(coverage
            DEPENDS unit_tests
            COMMAND "${OPENCPPCOVERAGE_NATIVE}"
                --verbose
                --sources "${COVERAGE_ROOT_PATH}"
                --modules "${COVERAGE_ROOT_PATH}"
                --excluded_sources "*\\tests\\*"
                --excluded_sources "*\\gtest\\*"
                --excluded_sources "*\\googletest\\*"
                --excluded_sources "*\\_deps\\*"
                --excluded_sources "*\\vcpkg_installed\\*"
                --export_type "html:${COVERAGE_OUTPUT_PATH}"
                --export_type "cobertura:${COVERAGE_XML_PATH}"
                --cover_children
                -- "$<TARGET_FILE:unit_tests>"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Running OpenCppCoverage"
        )

    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        # Linux/macOS: LLVM coverage via script
        add_custom_target(coverage
            DEPENDS unit_tests
            COMMAND bash "${CMAKE_SOURCE_DIR}/scripts/coverage-llvm.sh"
                "${CMAKE_SOURCE_DIR}"
                "${CMAKE_BINARY_DIR}"
                "$<TARGET_FILE:unit_tests>"
                "${CMAKE_BINARY_DIR}/coverage"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Running LLVM coverage"
        )

    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC: lcov/gcov
        add_custom_target(coverage
            DEPENDS unit_tests
            COMMAND $<TARGET_FILE:unit_tests> || true
            COMMAND lcov --capture --directory "${CMAKE_BINARY_DIR}"
                --output-file "${CMAKE_BINARY_DIR}/coverage.info"
                --ignore-errors mismatch,gcov
            COMMAND lcov --remove "${CMAKE_BINARY_DIR}/coverage.info"
                "*/tests/*" "*/gtest/*" "*/googletest/*" "*/_deps/*" "/usr/*"
                --output-file "${CMAKE_BINARY_DIR}/coverage.info"
                --ignore-errors unused
            COMMAND genhtml "${CMAKE_BINARY_DIR}/coverage.info"
                --output-directory "${CMAKE_BINARY_DIR}/coverage"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Running lcov/genhtml"
        )
    endif()
endfunction()
