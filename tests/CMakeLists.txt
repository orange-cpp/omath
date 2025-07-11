enable_testing()

project(unit_tests)

include(GoogleTest)

file(GLOB_RECURSE UNIT_TESTS_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(${PROJECT_NAME} ${UNIT_TESTS_SOURCES})

set_target_properties(unit_tests PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        UNITY_BUILD ON
        UNITY_BUILD_BATCH_SIZE 20
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON)


target_link_libraries(${PROJECT_NAME} PRIVATE gtest gtest_main omath::omath)

gtest_discover_tests(${PROJECT_NAME})