function(omath_setup_coverage_for_root OMATH_PROJECT)
    # Configure compilation properties for coverage helper translation units
    # (use project root paths so this behaves the same regardless of where
    # the file is included from).
    set(OMATH_SRC_DIR "${CMAKE_SOURCE_DIR}")
    set(OMATH_BIN_DIR "${CMAKE_BINARY_DIR}")

    # If forward_helpers.cpp is present, compile it with inlining disabled.
    if (EXISTS "${OMATH_SRC_DIR}/source/coverage/forward_helpers.cpp")
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
            set_source_files_properties(${OMATH_SRC_DIR}/source/coverage/forward_helpers.cpp PROPERTIES COMPILE_FLAGS "-fno-inline")
        elseif (MSVC)
            set_source_files_properties(${OMATH_SRC_DIR}/source/coverage/forward_helpers.cpp PROPERTIES COMPILE_FLAGS "/Ob0")
        endif()
    endif()

    # Always provide the coverage_coalescer tool target if the source exists.
    if (EXISTS "${OMATH_SRC_DIR}/tools/coverage_coalescer.cpp" AND NOT TARGET coverage_coalescer)
        add_executable(coverage_coalescer ${OMATH_SRC_DIR}/tools/coverage_coalescer.cpp)
        set_target_properties(coverage_coalescer PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools"
            OUTPUT_NAME "coverage_coalescer"
        )
    endif()

    # Also disable inlining for other coverage helper translation units.
    if (EXISTS "${OMATH_SRC_DIR}/source/coverage/linear_algebra_wrappers.cpp")
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
            set_source_files_properties(${OMATH_SRC_DIR}/source/coverage/linear_algebra_wrappers.cpp PROPERTIES COMPILE_FLAGS "-fno-inline")
        elseif (MSVC)
            set_source_files_properties(${OMATH_SRC_DIR}/source/coverage/linear_algebra_wrappers.cpp PROPERTIES COMPILE_FLAGS "/Ob0")
        endif()
    endif()

    if (EXISTS "${OMATH_SRC_DIR}/source/coverage/extra_linear_algebra_coverage.cpp")
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
            set_source_files_properties(${OMATH_SRC_DIR}/source/coverage/extra_linear_algebra_coverage.cpp PROPERTIES COMPILE_FLAGS "-fno-inline")
        elseif (MSVC)
            set_source_files_properties(${OMATH_SRC_DIR}/source/coverage/extra_linear_algebra_coverage.cpp PROPERTIES COMPILE_FLAGS "/Ob0")
        endif()
    endif()

    if (EXISTS "${OMATH_SRC_DIR}/source/coverage/explicit_instantiations.cpp")
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
            set_source_files_properties(${OMATH_SRC_DIR}/source/coverage/explicit_instantiations.cpp PROPERTIES COMPILE_FLAGS "-fno-inline")
        elseif (MSVC)
            set_source_files_properties(${OMATH_SRC_DIR}/source/coverage/explicit_instantiations.cpp PROPERTIES COMPILE_FLAGS "/Ob0")
        endif()
    endif()

    # Add additional debug-related flags to coverage-related TUs
    set(COVERAGE_TUS
        ${OMATH_SRC_DIR}/source/coverage/forward_helpers.cpp
        ${OMATH_SRC_DIR}/source/coverage/explicit_instantiations.cpp
        ${OMATH_SRC_DIR}/source/coverage/force_instantiations.cpp
        ${OMATH_SRC_DIR}/source/coverage/linear_algebra_wrappers.cpp
        ${OMATH_SRC_DIR}/source/coverage/extra_linear_algebra_coverage.cpp)

    foreach(_tu IN LISTS COVERAGE_TUS)
        if (EXISTS "${_tu}")
            if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
                set_source_files_properties(${_tu} PROPERTIES COMPILE_FLAGS "-g -O0 -fno-omit-frame-pointer -fno-inline")
            elseif (MSVC)
                set_source_files_properties(${_tu} PROPERTIES COMPILE_FLAGS "/Zo /Oy- /Ob0")
            endif()
        endif()
    endforeach()

    # Project-level coverage configuration (compiler/linker flags and coverage targets)
    if(CMAKE_HOST_LINUX AND OMATH_BUILD_TESTS)
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
            target_compile_options(${OMATH_PROJECT} PRIVATE
                $<$<CONFIG:Debug>:-g>
                $<$<CONFIG:Debug>:-fprofile-instr-generate>
                $<$<CONFIG:Debug>:-fcoverage-mapping>
            )
        else()
            target_compile_options(${OMATH_PROJECT} PRIVATE
                $<$<CONFIG:Debug>:-g>
                $<$<CONFIG:Debug>:-fprofile-arcs>
                $<$<CONFIG:Debug>:-ftest-coverage>
            )
            target_link_libraries(${OMATH_PROJECT} PRIVATE
                $<$<CONFIG:Debug>:-fprofile-arcs>
                $<$<CONFIG:Debug>:-ftest-coverage>
            )
        endif()

        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
            file(TO_CMAKE_PATH "${OMATH_SRC_DIR}" OMATH_SRC_PATH)
            file(TO_CMAKE_PATH "${OMATH_BIN_DIR}" OMATH_BUILD_PATH)
            string(REPLACE "/" "\\/" OMATH_SRC_PATH_ESCAPED "${OMATH_SRC_PATH}")
            string(REPLACE "/" "\\/" OMATH_BUILD_PATH_ESCAPED "${OMATH_BUILD_PATH}")

            target_compile_options(${OMATH_PROJECT} PRIVATE
                $<$<CONFIG:Debug>:-ffile-prefix-map=${OMATH_SRC_PATH}=..>
                $<$<CONFIG:Debug>:-ffile-prefix-map=${OMATH_BUILD_PATH}=.>
                $<$<CONFIG:Debug>:-fdebug-prefix-map=${OMATH_SRC_PATH}=..>
                $<$<CONFIG:Debug>:-fdebug-prefix-map=${OMATH_BUILD_PATH}=.>
            )
        endif()

        # Expose variables for coverage script substitution and configure script
        set(OMATH_LCOV_IGNORE_ERRORS "mismatch,inconsistent")
        set(OMATH_BR_EXCLUSION "LCOV_EXCL_BR_LINE|assert\\(")
        set(OMATH_LCOV_EXTRACT_PATHS "\"${OMATH_SRC_DIR}/include/omath/linear_algebra/*\" \"${OMATH_SRC_DIR}/include/omath/*\" \"${OMATH_SRC_DIR}/include/*\" \"${OMATH_SRC_DIR}/source/*\"")

        set(LCOV_IGNORE_ERRORS "${OMATH_LCOV_IGNORE_ERRORS}")
        set(BR_EXCLUSION "${OMATH_BR_EXCLUSION}")
        set(LCOV_EXTRACT_PATHS "${OMATH_LCOV_EXTRACT_PATHS}")

        configure_file(
            "${OMATH_SRC_DIR}/scripts/coverage.sh.in"
            "${OMATH_BIN_DIR}/scripts/coverage.sh"
            @ONLY)

        # Ensure the coverage_coalescer tool is available (guard already above).

        # Add coverage helper make clean files and targets
        set_property(
            DIRECTORY
            APPEND
            PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "coverage.info")
        set_property(
            DIRECTORY
            APPEND
            PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "coverage.cleaned.info")
        set_property(
            DIRECTORY
            APPEND
            PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "coverage")
        add_custom_target(
            coverage
            DEPENDS unit_tests
            COMMAND bash "${OMATH_BIN_DIR}/scripts/coverage.sh"
                "${OMATH_SRC_DIR}" "${OMATH_BIN_DIR}"
            WORKING_DIRECTORY "${OMATH_BIN_DIR}"
            COMMENT "Run coverage: run tests, collect via gcov, postprocess and generate HTML")
        add_custom_target(
            clean-coverage
            COMMAND lcov --rc branch_coverage=1 --directory
                    '${OMATH_BIN_DIR}' --zerocounters
            COMMENT "Zeroing counters")
    endif()
endfunction()

function(omath_setup_coverage_for_test TEST_TARGET)
    # Called from tests/CMakeLists; CMAKE_CURRENT_SOURCE_DIR will be the tests/ dir
    set(TEST_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

    # Compile specific TU without inlining to help coverage attribute header lines
    set_source_files_properties(
        ${TEST_SRC_DIR}/general/coverage_instantiations.cpp
        ${TEST_SRC_DIR}/general/coverage_utility_instantiations.cpp
        PROPERTIES COMPILE_OPTIONS "-fno-inline")

    # If building with Clang and coverage enabled, remove any GCC-style
    # coverage link flags that might have been propagated to the test target
    # so that Clang's instrumentation flags control the coverage output.
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        string(REPLACE "-fprofile-arcs" "" _tmp_link_opts "${CMAKE_SHARED_LINKER_FLAGS}")
        string(REPLACE "-ftest-coverage" "" _tmp_link_opts "${_tmp_link_opts}")
        set_target_properties(${TEST_TARGET} PROPERTIES LINK_FLAGS "${_tmp_link_opts}")
        target_compile_options(${TEST_TARGET} PRIVATE
            $<$<CONFIG:Debug>:-fprofile-instr-generate>
            $<$<CONFIG:Debug>:-fcoverage-mapping>
            $<$<CONFIG:Debug>:-g>
        )
        if(EMSCRIPTEN)
            target_compile_options(${TEST_TARGET} PRIVATE -fexceptions)
            target_link_options(${TEST_TARGET} PRIVATE -fexceptions)
        endif()
        if (CMAKE_VERSION VERSION_GREATER "3.13")
            target_link_options(${TEST_TARGET} PRIVATE $<$<CONFIG:Debug>:-fprofile-instr-generate> $<$<CONFIG:Debug>:-fcoverage-mapping>)
        else()
            string(APPEND _tmp_link_flags " -fprofile-instr-generate -fcoverage-mapping")
            set_target_properties(${TEST_TARGET} PROPERTIES LINK_FLAGS "${_tmp_link_flags}")
        endif()
    endif()

    # Discover tests except on platforms where test binaries cannot run
    if (NOT (ANDROID OR IOS OR EMSCRIPTEN))
        include(GoogleTest)
        gtest_discover_tests(${TEST_TARGET})
    endif()
endfunction()
