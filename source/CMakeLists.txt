target_sources(omath PRIVATE
        Vector3.cpp
        Matrix.cpp
        color.cpp
        Vector4.cpp
        Vector2.cpp
)

add_subdirectory(projectile_prediction)
add_subdirectory(pathfinding)
add_subdirectory(projection)
add_subdirectory(collision)
add_subdirectory(engines)