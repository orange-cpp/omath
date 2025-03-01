target_sources(omath PRIVATE
        Matrix.cpp
        color.cpp
)

add_subdirectory(projectile_prediction)
add_subdirectory(pathfinding)
add_subdirectory(projection)
add_subdirectory(collision)
add_subdirectory(engines)