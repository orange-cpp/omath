project(examples)

add_executable(example_projection_matrix_builder example_proj_mat_builder.cpp)
set_target_properties(
    example_projection_matrix_builder
    PROPERTIES CXX_STANDARD 23
               ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}")
target_link_libraries(example_projection_matrix_builder PRIVATE omath::omath)

add_executable(example_signature_scan example_signature_scan.cpp)
set_target_properties(
    example_signature_scan
    PROPERTIES CXX_STANDARD 23
               ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}")
target_link_libraries(example_signature_scan PRIVATE omath::omath)

add_executable(example_glfw3 example_glfw3.cpp)
set_target_properties(
    example_glfw3
    PROPERTIES CXX_STANDARD 23
               ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}")

find_package(OpenGL)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(example_glfw3 PRIVATE omath::omath GLEW::GLEW glfw OpenGL::GL)

if(OMATH_ENABLE_VALGRIND)
    omath_setup_valgrind(example_projection_matrix_builder)
    omath_setup_valgrind(example_signature_scan)
    omath_setup_valgrind(example_glfw3)
endif()
