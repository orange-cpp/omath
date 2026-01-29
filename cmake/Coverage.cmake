# cmake/Coverage.cmake
include_guard(GLOBAL)

function(omath_setup_coverage TARGET_NAME)
    if(ANDROID OR IOS OR EMSCRIPTEN)
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping
                                                      /Zi)
        target_link_options(
            ${TARGET_NAME}
            PRIVATE
            -fprofile-instr-generate
            -fcoverage-mapping
            /DEBUG:FULL
            /INCREMENTAL:NO
            /PROFILE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        target_compile_options(${TARGET_NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping
                                                      -g -O0)
        target_link_options(${TARGET_NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${TARGET_NAME} PRIVATE --coverage -g -O0)
        target_link_options(${TARGET_NAME} PRIVATE --coverage)
    endif()

    if(TARGET coverage)
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND MSVC)
        message(STATUS "MSVC detected: Use VS Code Coverage from CI workflow")
        add_custom_target(
            coverage
            DEPENDS unit_tests
            COMMAND $<TARGET_FILE:unit_tests>
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Running tests for coverage (use VS Code Coverage from CI)")

    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        add_custom_target(
            coverage
            DEPENDS unit_tests
            COMMAND bash "${CMAKE_SOURCE_DIR}/scripts/coverage-llvm.sh" "${CMAKE_SOURCE_DIR}"
                    "${CMAKE_BINARY_DIR}" "$<TARGET_FILE:unit_tests>" "${CMAKE_BINARY_DIR}/coverage"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Running LLVM coverage")

    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_custom_target(
            coverage
            DEPENDS unit_tests
            COMMAND $<TARGET_FILE:unit_tests> || true
            COMMAND lcov --capture --directory "${CMAKE_BINARY_DIR}" --output-file
                    "${CMAKE_BINARY_DIR}/coverage.info" --ignore-errors mismatch,gcov
            COMMAND
                lcov --remove "${CMAKE_BINARY_DIR}/coverage.info" "*/tests/*" "*/gtest/*"
                "*/googletest/*" "*/_deps/*" "/usr/*" --output-file
                "${CMAKE_BINARY_DIR}/coverage.info" --ignore-errors unused
            COMMAND genhtml "${CMAKE_BINARY_DIR}/coverage.info" --output-directory
                    "${CMAKE_BINARY_DIR}/coverage"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Running lcov/genhtml")
    endif()
endfunction()
