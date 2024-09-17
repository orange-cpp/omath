enable_testing()

project(unit-tests)

include(GoogleTest)
add_executable(unit-tests
        UnitTestPrediction.cpp
        UnitTestMatrix.cpp
        UnitTestAstar.cpp
        UnitTestProjection.cpp
        UnitTestVector3.cpp
        UnitTestVector2.cpp
        UnitTestColor.cpp
        UnitTestHooking.cpp
)

target_link_libraries(unit-tests PRIVATE gtest gtest_main omath)

gtest_discover_tests(unit-tests)