enable_testing()

project(unit-tests)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}")

include(GoogleTest)
add_executable(unit-tests
        UnitTestPrediction.cpp
        UnitTestMatrix.cpp
        UnitTestMat.cpp
        UnitTestAstar.cpp
        UnitTestProjection.cpp
        UnitTestVector3.cpp
        UnitTestVector2.cpp
        UnitTestColor.cpp
        UnitTestVector4.cpp)

target_link_libraries(unit-tests PRIVATE gtest gtest_main omath)

gtest_discover_tests(unit-tests)