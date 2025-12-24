# Best Practices

Guidelines for using OMath effectively and avoiding common pitfalls.

---

## Code Organization

### Use Type Aliases

Define clear type aliases for commonly used types:

```cpp
// Good: Clear and concise
using Vec3f = omath::Vector3<float>;
using Vec2f = omath::Vector2<float>;
using Mat4 = omath::Mat4X4;

Vec3f position{1.0f, 2.0f, 3.0f};
```

```cpp
// Avoid: Verbose and repetitive
omath::Vector3<float> position{1.0f, 2.0f, 3.0f};
omath::Vector3<float> velocity{0.0f, 0.0f, 0.0f};
```

### Namespace Usage

Be selective with `using namespace`:

```cpp
// Good: Specific namespace for your engine
using namespace omath::source_engine;

// Good: Import specific types
using omath::Vector3;
using omath::Vector2;

// Avoid: Too broad
using namespace omath;  // Imports everything
```

### Include What You Use

```cpp
// Good: Include specific headers
#include <omath/linear_algebra/vector3.hpp>
#include <omath/projection/camera.hpp>

// Okay for development
#include <omath/omath.hpp>

// Production: Include only what you need
// to reduce compile times
```

---

## Error Handling

### Always Check Optional Results

```cpp
// Good: Check before using
if (auto screen = camera.world_to_screen(world_pos)) {
    draw_at(screen->x, screen->y);
} else {
    // Handle point not visible
}

// Bad: Unchecked access can crash
auto screen = camera.world_to_screen(world_pos);
draw_at(screen->x, screen->y);  // Undefined behavior if nullopt!
```

### Handle Expected Errors

```cpp
// Good: Handle error case
if (auto angle = v1.angle_between(v2)) {
    use_angle(*angle);
} else {
    switch (angle.error()) {
        case Vector3Error::IMPOSSIBLE_BETWEEN_ANGLE:
            // Handle zero-length vector
            break;
    }
}

// Bad: Assume success
auto angle = v1.angle_between(v2);
use_angle(*angle);  // Throws if error!
```

### Validate Inputs

```cpp
// Good: Validate before expensive operations
bool is_valid_projectile(const Projectile& proj) {
    return proj.speed > 0.0f &&
           std::isfinite(proj.speed) &&
           std::isfinite(proj.origin.length());
}

if (is_valid_projectile(proj) && is_valid_target(target)) {
    auto aim = engine.maybe_calculate_aim_point(proj, target);
}
```

---

## Performance

### Use constexpr When Possible

```cpp
// Good: Computed at compile time
constexpr Vector3<float> gravity{0.0f, 0.0f, -9.81f};
constexpr float max_range = 1000.0f;
constexpr float max_range_sq = max_range * max_range;

// Use in runtime calculations
if (position.length_sqr() < max_range_sq) {
    // ...
}
```

### Prefer Squared Distance

```cpp
// Good: Avoids expensive sqrt
constexpr float max_dist_sq = 100.0f * 100.0f;
for (const auto& entity : entities) {
    if (entity.pos.distance_to_sqr(player_pos) < max_dist_sq) {
        // Process nearby entity
    }
}

// Avoid: Unnecessary sqrt calls
constexpr float max_dist = 100.0f;
for (const auto& entity : entities) {
    if (entity.pos.distance_to(player_pos) < max_dist) {
        // More expensive
    }
}
```

### Cache Expensive Calculations

```cpp
// Good: Update camera once per frame
void update_frame() {
    camera.update(current_position, current_angles);
    
    // All projections use cached matrices
    for (const auto& entity : entities) {
        if (auto screen = camera.world_to_screen(entity.pos)) {
            draw_entity(screen->x, screen->y);
        }
    }
}

// Bad: Camera recreated each call
for (const auto& entity : entities) {
    Camera cam(pos, angles, viewport, fov, near, far);  // Expensive!
    auto screen = cam.world_to_screen(entity.pos);
}
```

### Choose the Right Engine

```cpp
// Good: Use AVX2 when available
#ifdef __AVX2__
    using Engine = ProjPredEngineAVX2;
#else
    using Engine = ProjPredEngineLegacy;
#endif

Engine prediction_engine;

// Or runtime detection
Engine* create_best_engine() {
    if (cpu_supports_avx2()) {
        return new ProjPredEngineAVX2();
    }
    return new ProjPredEngineLegacy();
}
```

### Minimize Allocations

```cpp
// Good: Reuse vectors
std::vector<Vector3<float>> positions;
positions.reserve(expected_count);

// In loop
positions.clear();  // Doesn't deallocate
for (...) {
    positions.push_back(compute_position());
}

// Bad: Allocate every time
for (...) {
    std::vector<Vector3<float>> positions;  // Allocates each iteration
    // ...
}
```

---

## Type Safety

### Use Strong Angle Types

```cpp
// Good: Type-safe angles
PitchAngle pitch = PitchAngle::from_degrees(45.0f);
YawAngle yaw = YawAngle::from_degrees(90.0f);
ViewAngles angles{pitch, yaw, RollAngle::from_degrees(0.0f)};

// Bad: Raw floats lose safety
float pitch = 45.0f;  // No range checking
float yaw = 90.0f;    // Can go out of bounds
```

### Match Engine Types

```cpp
// Good: Use matching types from same engine
using namespace omath::source_engine;
Camera camera = /* ... */;
ViewAngles angles = /* ... */;

// Bad: Mixing engine types
using UnityCamera = omath::unity_engine::Camera;
using SourceAngles = omath::source_engine::ViewAngles;
UnityCamera camera{pos, SourceAngles{}, ...};  // May cause issues!
```

### Template Type Parameters

```cpp
// Good: Explicit and clear
Vector3<float> position;
Vector3<double> high_precision_pos;

// Okay: Use default float
Vector3<> position;  // Defaults to float

// Avoid: Mixing types unintentionally
Vector3<float> a;
Vector3<double> b;
auto result = a + b;  // Type mismatch!
```

---

## Testing & Validation

### Test Edge Cases

```cpp
void test_projection() {
    Camera camera = setup_camera();
    
    // Test normal case
    assert(camera.world_to_screen({100, 100, 100}).has_value());
    
    // Test edge cases
    assert(!camera.world_to_screen({0, 0, -100}).has_value());  // Behind
    assert(!camera.world_to_screen({1e10, 0, 0}).has_value());  // Too far
    
    // Test boundaries
    Vector3<float> at_near{0, 0, camera.near_plane() + 0.1f};
    assert(camera.world_to_screen(at_near).has_value());
}
```

### Validate Assumptions

```cpp
void validate_game_data() {
    // Validate FOV
    float fov = read_game_fov();
    assert(fov > 1.0f && fov < 179.0f);
    
    // Validate positions
    Vector3<float> pos = read_player_position();
    assert(std::isfinite(pos.x));
    assert(std::isfinite(pos.y));
    assert(std::isfinite(pos.z));
    
    // Validate viewport
    ViewPort vp = read_viewport();
    assert(vp.width > 0 && vp.height > 0);
}
```

### Use Assertions

```cpp
// Good: Catch errors early in development
void shoot_projectile(const Projectile& proj) {
    assert(proj.speed > 0.0f && "Projectile speed must be positive");
    assert(std::isfinite(proj.origin.length()) && "Invalid projectile origin");
    
    // Continue with logic
}

// Add debug-only checks
#ifndef NDEBUG
    if (!is_valid_input(data)) {
        std::cerr << "Warning: Invalid input detected\n";
    }
#endif
```

---

## Memory & Resources

### RAII for Resources

```cpp
// Good: Automatic cleanup
class GameOverlay {
    Camera camera_;
    std::vector<Entity> entities_;
    
public:
    GameOverlay(/* ... */) : camera_(/* ... */) {
        entities_.reserve(1000);
    }
    
    // Resources cleaned up automatically
    ~GameOverlay() = default;
};
```

### Avoid Unnecessary Copies

```cpp
// Good: Pass by const reference
void draw_entities(const std::vector<Vector3<float>>& positions) {
    for (const auto& pos : positions) {
        // Process position
    }
}

// Bad: Copies entire vector
void draw_entities(std::vector<Vector3<float>> positions) {
    // Expensive copy!
}

// Good: Move when transferring ownership
std::vector<Vector3<float>> compute_positions();
auto positions = compute_positions();  // Move, not copy
```

### Use Structured Bindings

```cpp
// Good: Clear and concise
if (auto [success, screen_pos] = try_project(world_pos); success) {
    draw_at(screen_pos.x, screen_pos.y);
}

// Good: Decompose results
auto [x, y, z] = position.as_tuple();
```

---

## Documentation

### Document Assumptions

```cpp
// Good: Clear documentation
/**
 * Projects world position to screen space.
 * 
 * @param world_pos Position in world coordinates (meters)
 * @return Screen position if visible, nullopt if behind camera or out of view
 * 
 * @note Assumes camera.update() was called this frame
 * @note Screen coordinates are in viewport space [0, width] x [0, height]
 */
std::optional<Vector2<float>> project(const Vector3<float>& world_pos);
```

### Explain Non-Obvious Code

```cpp
// Good: Explain the math
// Use squared distance to avoid expensive sqrt
// max_range = 100.0 → max_range_sq = 10000.0
constexpr float max_range_sq = 100.0f * 100.0f;
if (dist_sq < max_range_sq) {
    // Entity is in range
}

// Explain engine-specific quirks
// Source Engine uses Z-up coordinates, but angles are in degrees
// Pitch: [-89, 89], Yaw: [-180, 180], Roll: [-180, 180]
ViewAngles angles{pitch, yaw, roll};
```

---

## Debugging

### Add Debug Visualization

```cpp
#ifndef NDEBUG
void debug_draw_projection() {
    // Draw camera frustum
    draw_frustum(camera);
    
    // Draw world axes
    draw_line({0,0,0}, {100,0,0}, Color::Red);    // X
    draw_line({0,0,0}, {0,100,0}, Color::Green);  // Y
    draw_line({0,0,0}, {0,0,100}, Color::Blue);   // Z
    
    // Draw projected points
    for (const auto& entity : entities) {
        if (auto screen = camera.world_to_screen(entity.pos)) {
            draw_cross(screen->x, screen->y);
        }
    }
}
#endif
```

### Log Important Values

```cpp
void debug_projection_failure(const Vector3<float>& pos) {
    std::cerr << "Projection failed for position: "
              << pos.x << ", " << pos.y << ", " << pos.z << "\n";
    
    auto view_matrix = camera.get_view_matrix();
    std::cerr << "View matrix:\n";
    // Print matrix...
    
    std::cerr << "Camera position: "
              << camera.position().x << ", "
              << camera.position().y << ", "
              << camera.position().z << "\n";
}
```

### Use Debug Builds

```cmake
# CMakeLists.txt
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(your_target PRIVATE
        DEBUG_PROJECTION=1
        VALIDATE_INPUTS=1
    )
endif()
```

```cpp
#ifdef DEBUG_PROJECTION
    std::cout << "Projecting: " << world_pos << "\n";
#endif

#ifdef VALIDATE_INPUTS
    assert(std::isfinite(world_pos.length()));
#endif
```

---

## Platform Considerations

### Cross-Platform Code

```cpp
// Good: Platform-agnostic
constexpr float PI = 3.14159265359f;

// Avoid: Platform-specific
#ifdef _WIN32
    // Windows-only code
#endif
```

### Handle Different Compilers

```cpp
// Good: Compiler-agnostic
#if defined(_MSC_VER)
    // MSVC-specific
#elif defined(__GNUC__)
    // GCC/Clang-specific
#endif

// Use OMath's built-in compatibility
// It handles compiler differences automatically
```

---

## Summary

**Key principles:**
1. **Safety first**: Always check optional/expected results
2. **Performance matters**: Use constexpr, avoid allocations, cache results
3. **Type safety**: Use strong types, match engine types
4. **Clear code**: Use aliases, document assumptions, explain non-obvious logic
5. **Test thoroughly**: Validate inputs, test edge cases, add assertions
6. **Debug effectively**: Add visualization, log values, use debug builds

---

## See Also

- [Troubleshooting Guide](troubleshooting.md)
- [FAQ](faq.md)
- [API Overview](api_overview.md)
- [Tutorials](tutorials.md)

---

*Last updated: 24 Dec 2025*
