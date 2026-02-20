project(example_barycentric)

add_executable(${PROJECT_NAME} example_barycentric.cpp)
set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES CXX_STANDARD 23
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}")

find_package(OpenGL)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE omath::omath GLEW::GLEW glfw OpenGL::GL)