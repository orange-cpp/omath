project(example_signature_scan)

add_executable(${PROJECT_NAME} example_signature_scan.cpp)
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES CXX_STANDARD 23
               ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}")
target_link_libraries(${PROJECT_NAME} PRIVATE omath::omath)
