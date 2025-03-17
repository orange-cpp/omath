enable_testing()

project(unit-tests)

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
        general/UnitTestViewAngles.cpp
        general/UnitTestAngle.cpp
        general/UnitTestTriangle.cpp

        engines/UnitTestOpenGL.cpp
        engines/UnitTestUnityEngine.cpp
        engines/UnitTestSourceEngine.cpp
        engines/UnitTestIwEngine.cpp

)

set_target_properties(unit-tests PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        UNITY_BUILD ON
        UNITY_BUILD_BATCH_SIZE 20
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON)


target_link_libraries(unit-tests PRIVATE gtest gtest_main omath)

gtest_discover_tests(unit-tests)