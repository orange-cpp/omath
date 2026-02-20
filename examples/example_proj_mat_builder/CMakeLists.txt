project(example_projection_matrix_builder)

add_executable(${PROJECT_NAME} example_proj_mat_builder.cpp)
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES CXX_STANDARD 23
               ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}")
target_link_libraries(${PROJECT_NAME} PRIVATE omath::omath)
