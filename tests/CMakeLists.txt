enable_testing()

project(unit-tests)

include(GoogleTest)
add_executable(unit-tests
        general/unit_test_prediction.cpp
        general/unit_test_matrix.cpp
        general/unit_test_mat.cpp
        general/unit_test_a_star.cpp
        general/unit_test_projection.cpp
        general/unit_test_vector3.cpp
        general/unit_test_vector2.cpp
        general/unit_test_color.cpp
        general/unit_test_vector4.cpp
        general/unit_test_line_trace.cpp
        general/unit_test_angles.cpp
        general/unit_test_view_angles.cpp
        general/unit_test_angle.cpp
        general/unit_test_triangle.cpp

        engines/unit_test_open_gl.cpp
        engines/unit_test_unity_engine.cpp
        engines/unit_test_source_engine.cpp
        engines/unit_test_iw_engine.cpp

)

set_target_properties(unit-tests PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        UNITY_BUILD ON
        UNITY_BUILD_BATCH_SIZE 20
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON)


target_link_libraries(unit-tests PRIVATE gtest gtest_main omath::omath)

gtest_discover_tests(unit-tests)