enable_testing()

project(unit-tests)

file(GLOB TEST_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

include(GoogleTest)
add_executable(unit-tests UnitTestColor.cpp UnitTestMatrix.cpp)

target_link_libraries(unit-tests PRIVATE gtest gtest_main omath)

gtest_discover_tests(unit-tests)