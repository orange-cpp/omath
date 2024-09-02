target_sources(omath PRIVATE
        Vector3.cpp
        matrix.cpp
        color.cpp
        Vector4.cpp
        Vector2.cpp)

add_subdirectory(prediction)
add_subdirectory(pathfinding)
add_subdirectory(projection)