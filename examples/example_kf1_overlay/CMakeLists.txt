project(example_kf1_overlay)

add_executable(${PROJECT_NAME} example_kf1_overlay.cpp)
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES CXX_STANDARD 23
               ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}")

# kf1_common/kf1_target.hpp is shared with example_kf1_dumper
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE omath::omath glfw imgui::imgui OpenGL::GL)
