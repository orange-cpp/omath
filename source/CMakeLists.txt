target_sources(omath PRIVATE
        Vector3.cpp
        Matrix.cpp
        color.cpp
        Vector4.cpp
        Vector2.cpp
        Triangle3d.cpp
)

add_subdirectory(prediction)
add_subdirectory(pathfinding)
add_subdirectory(projection)
add_subdirectory(collision)
add_subdirectory(engines)