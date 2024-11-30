enable_testing()

project(unit-tests)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}")

include(GoogleTest)
add_executable(unit-tests
        general/UnitTestPrediction.cpp
        general/UnitTestMatrix.cpp
        general/UnitTestMat.cpp
        general/UnitTestAstar.cpp
        general/UnitTestProjection.cpp
        general/UnitTestVector3.cpp
        general/UnitTestVector2.cpp
        general/UnitTestColor.cpp
        general/UnitTestVector4.cpp
        general/UnitTestLineTrace.cpp
        general/UnitTestAngles.cpp

        engines/UnitTestOpenGL.cpp
        engines/UnitTestUnityEngine.cpp
        engines/UnitTestSourceEngine.cpp

)

target_link_libraries(unit-tests PRIVATE gtest gtest_main omath glm)

gtest_discover_tests(unit-tests)