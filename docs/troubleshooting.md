# Troubleshooting Guide

Solutions to common problems when using OMath.

---

## Build & Compilation Issues

### Error: C++20 features not available

**Problem:** Compiler doesn't support C++20.

**Solution:**
Upgrade your compiler:
- **GCC**: Version 10 or newer
- **Clang**: Version 11 or newer  
- **MSVC**: Visual Studio 2019 16.10 or newer

Set C++20 in CMakeLists.txt:
```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

### Error: `std::expected` not found

**Problem:** Using C++20 without C++23's `std::expected`.

**Solutions:**

1. **Upgrade to C++23** (recommended):
   ```cmake
   set(CMAKE_CXX_STANDARD 23)
   ```

2. **Use a backport library**:
   ```cmake
   find_package(tl-expected CONFIG REQUIRED)
   target_link_libraries(your_target PRIVATE tl::expected)
   ```

### Error: `omath/omath.hpp` not found

**Problem:** OMath not installed or not in include path.

**Solution:**

Check installation:
```bash
# vcpkg
vcpkg list | grep omath

# Check if files exist
ls /path/to/vcpkg/installed/x64-linux/include/omath
```

In CMakeLists.txt:
```cmake
find_package(omath CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE omath::omath)
```

### Linker errors with AVX2 engine

**Problem:** Undefined references to AVX2 functions.

**Solution:**

Enable AVX2 in your build:
```cmake
if(MSVC)
    target_compile_options(your_target PRIVATE /arch:AVX2)
else()
    target_compile_options(your_target PRIVATE -mavx2)
endif()
```

Or use the legacy engine instead:
```cpp
// Use this instead of ProjPredEngineAVX2
ProjPredEngineLegacy engine;
```

---

## Runtime Issues

### `world_to_screen()` always returns `nullopt`

**Common causes:**

1. **Point behind camera**
   ```cpp
   // Point is behind the camera
   Vector3<float> behind = camera_pos - Vector3<float>{0, 0, 100};
   auto result = camera.world_to_screen(behind);  // Returns nullopt
   ```
   
   **Fix:** Only project points in front of camera. Check Z-coordinate in view space.

2. **Invalid near/far planes**
   ```cpp
   // Bad: near >= far
   Camera camera(pos, angles, viewport, fov, 100.0f, 1.0f);
   
   // Good: near < far
   Camera camera(pos, angles, viewport, fov, 0.1f, 1000.0f);
   ```

3. **Invalid FOV**
   ```cpp
   // Bad: FOV out of range
   auto fov = FieldOfView::from_degrees(0.0f);   // Too small
   auto fov = FieldOfView::from_degrees(180.0f); // Too large
   
   // Good: FOV in valid range
   auto fov = FieldOfView::from_degrees(90.0f);
   ```

4. **Uninitialized camera**
   ```cpp
   // Make sure camera is properly initialized
   camera.update(current_position, current_angles);
   ```

**Debugging:**
```cpp
Vector3<float> world_pos{100, 100, 100};

// Check projection step by step
std::cout << "World pos: " << world_pos.x << ", " 
          << world_pos.y << ", " << world_pos.z << "\n";

auto view_matrix = camera.get_view_matrix();
// Transform to view space manually and check if Z > 0

if (auto screen = camera.world_to_screen(world_pos)) {
    std::cout << "Success: " << screen->x << ", " << screen->y << "\n";
} else {
    std::cout << "Failed - check if point is behind camera\n";
}
```

### Angles wrapping incorrectly

**Problem:** Angles not normalizing to expected ranges.

**Solution:**

Use proper angle types:
```cpp
// Wrong: using raw floats
float pitch = 95.0f;  // Out of valid range!

// Right: using typed angles
auto pitch = PitchAngle::from_degrees(89.0f);  // Clamped to valid range
```

For custom ranges:
```cpp
// Define custom angle with wrapping
auto angle = Angle<float, -180.0f, 180.0f, AngleFlags::Normalized>::from_degrees(270.0f);
// Result: -90° (wrapped)
```

### Projection appears mirrored or inverted

**Problem:** Using wrong engine trait for your game.

**Solution:**

Different engines have different coordinate systems:

| Symptom | Likely Issue | Fix |
|---------|-------------|-----|
| Upside down | Y-axis inverted | Try different engine or negate Y |
| Left-right flipped | Wrong handedness | Check engine documentation |
| Rotated 90° | Axis swap | Verify engine coordinate system |

```cpp
// Try different engine traits
using namespace omath::source_engine;  // Z-up, left-handed
using namespace omath::unity_engine;   // Y-up, left-handed  
using namespace omath::unreal_engine;  // Z-up, left-handed (different conventions)
using namespace omath::opengl_engine;  // Y-up, right-handed
```

If still wrong, manually transform coordinates:
```cpp
// Example: swap Y and Z for Y-up to Z-up conversion
Vector3<float> convert_y_up_to_z_up(const Vector3<float>& pos) {
    return Vector3<float>{pos.x, pos.z, pos.y};
}
```

---

## Projectile Prediction Issues

### `maybe_calculate_aim_point()` returns `nullopt`

**Common causes:**

1. **Target moving too fast**
   ```cpp
   Target target;
   target.velocity = Vector3<float>{1000, 0, 0};  // Very fast!
   
   Projectile proj;
   proj.speed = 500.0f;  // Too slow to catch target
   
   // Returns nullopt - projectile can't catch target
   ```
   
   **Fix:** Check if projectile speed > target speed in the direction of motion.

2. **Zero projectile speed**
   ```cpp
   Projectile proj;
   proj.speed = 0.0f;  // Invalid!
   
   // Returns nullopt
   ```
   
   **Fix:** Ensure `proj.speed > 0`.

3. **Invalid positions**
   ```cpp
   // NaN or infinite values
   target.position = Vector3<float>{NAN, 0, 0};
   
   // Returns nullopt
   ```
   
   **Fix:** Validate all input values are finite.

4. **Target out of range**
   ```cpp
   // Target very far away
   float distance = shooter_pos.distance_to(target.position);
   float max_range = proj.speed * max_flight_time;
   
   if (distance > max_range) {
       // Will return nullopt
   }
   ```

**Debugging:**
```cpp
Projectile proj{/* ... */};
Target target{/* ... */};

// Check inputs
assert(proj.speed > 0);
assert(std::isfinite(target.position.length()));
assert(std::isfinite(target.velocity.length()));

// Check if target is reachable
float distance = proj.origin.distance_to(target.position);
float target_speed = target.velocity.length();

std::cout << "Distance: " << distance << "\n";
std::cout << "Projectile speed: " << proj.speed << "\n";
std::cout << "Target speed: " << target_speed << "\n";

if (target_speed >= proj.speed) {
    std::cout << "Target may be too fast!\n";
}
```

### Aim point is inaccurate

**Problem:** Calculated aim point doesn't hit target.

**Possible causes:**

1. **Unit mismatch**
   ```cpp
   // All units must match!
   proj.speed = 800.0f;  // meters per second
   target.velocity = Vector3<float>{2, 1, 0};  // Must also be m/s!
   
   // If using different units (e.g., game units vs meters), convert:
   float game_units_to_meters = 0.01905f;  // Example for Source
   target.velocity = game_velocity * game_units_to_meters;
   ```

2. **Wrong gravity vector**
   ```cpp
   // Source Engine: Z-up
   proj.gravity = Vector3<float>{0, 0, -9.81f};
   
   // Unity: Y-up
   proj.gravity = Vector3<float>{0, -9.81f, 0};
   ```

3. **Target velocity not updated**
   ```cpp
   // Update target velocity each frame
   target.velocity = current_velocity;  // Not last frame's velocity!
   ```

---

## Pattern Scanning Issues

### Pattern not found when it should be

**Problem:** `pattern_scan()` returns `nullopt` but pattern exists.

**Solutions:**

1. **Pattern syntax error**
   ```cpp
   // Wrong: missing spaces
   PatternView pattern{"488B05????????"};
   
   // Right: spaces between bytes
   PatternView pattern{"48 8B 05 ?? ?? ?? ??"};
   ```

2. **Pattern too specific**
   ```cpp
   // May fail if any byte is different
   PatternView pattern{"48 8B 05 01 02 03 04 48 85 C0"};
   
   // Better: use wildcards for variable bytes
   PatternView pattern{"48 8B 05 ?? ?? ?? ?? 48 85 C0"};
   ```

3. **Searching wrong memory region**
   ```cpp
   // Make sure you're scanning the right memory
   std::vector<uint8_t> code_section = get_code_section();
   auto result = pattern_scan(code_section, pattern);
   ```

4. **Pattern might have multiple matches**
   ```cpp
   // Find all matches instead of just first
   size_t offset = 0;
   while (offset < memory.size()) {
       auto result = pattern_scan(
           std::span(memory.begin() + offset, memory.end()),
           pattern
       );
       if (result) {
           std::cout << "Match at: " << offset + result->offset << "\n";
           offset += result->offset + 1;
       } else {
           break;
       }
   }
   ```

### Pattern found at wrong location

**Problem:** Pattern matches unintended code.

**Solutions:**

1. **Make pattern more specific**
   ```cpp
   // Too generic
   PatternView pattern{"48 8B"};
   
   // More specific - include more context
   PatternView pattern{"48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ??"};
   ```

2. **Verify found address**
   ```cpp
   if (auto result = pattern_scan(memory, pattern)) {
       // Verify by checking nearby bytes
       size_t offset = result->offset;
       
       // Check if instruction makes sense
       if (memory[offset] == 0x48 && memory[offset + 1] == 0x8B) {
           // Looks good
       }
   }
   ```

3. **Use multiple patterns**
   ```cpp
   // Find reference function first
   auto ref_pattern = PatternView{"E8 ?? ?? ?? ?? 85 C0"};
   auto ref_result = pattern_scan(memory, ref_pattern);
   
   // Then search near that location
   // This provides context validation
   ```

---

## Vector & Math Issues

### `normalized()` returns zero vector

**Problem:** Normalizing a zero-length vector.

**Behavior:**
```cpp
Vector3<float> zero{0, 0, 0};
auto result = zero.normalized();  // Returns {0, 0, 0}
```

This is **intentional** to avoid NaN. Check vector length first:
```cpp
if (v.length() > 0.001f) {
    auto normalized = v.normalized();
    // Use normalized vector
} else {
    // Handle zero-length case
}
```

### `angle_between()` returns error

**Problem:** One or both vectors have zero length.

**Solution:**
```cpp
auto angle_result = v1.angle_between(v2);

if (angle_result) {
    float degrees = angle_result->as_degrees();
} else {
    // Handle error - one or both vectors have zero length
    std::cerr << "Cannot compute angle between zero-length vectors\n";
}
```

### Cross product seems wrong

**Problem:** Unexpected cross product result.

**Check:**
1. **Right-handed system**
   ```cpp
   Vector3<float> x{1, 0, 0};
   Vector3<float> y{0, 1, 0};
   auto z = x.cross(y);  // Should be {0, 0, 1} in right-handed system
   ```

2. **Order matters**
   ```cpp
   auto cross1 = a.cross(b);  // {x1, y1, z1}
   auto cross2 = b.cross(a);  // {-x1, -y1, -z1} (opposite direction!)
   ```

---

## Performance Issues

### Code is slower than expected

**Solutions:**

1. **Enable optimizations**
   ```cmake
   # CMakeLists.txt
   target_compile_options(your_target PRIVATE
       $<$<CONFIG:Release>:-O3>
       $<$<CONFIG:Release>:-march=native>
   )
   ```

2. **Use AVX2 engine**
   ```cpp
   // Instead of
   ProjPredEngineLegacy engine;
   
   // Use
   ProjPredEngineAVX2 engine;
   ```

3. **Avoid unnecessary operations**
   ```cpp
   // Bad: recompute every frame
   for (auto& entity : entities) {
       float dist = entity.pos.distance_to(player_pos);  // Expensive sqrt!
       if (dist < 100.0f) { /* ... */ }
   }
   
   // Good: use squared distance
   constexpr float max_dist_sq = 100.0f * 100.0f;
   for (auto& entity : entities) {
       float dist_sq = entity.pos.distance_to_sqr(player_pos);  // No sqrt!
       if (dist_sq < max_dist_sq) { /* ... */ }
   }
   ```

4. **Cache matrices**
   ```cpp
   // Bad: recompute matrix every call
   for (auto& pos : positions) {
       auto screen = camera.world_to_screen(pos);  // Recomputes matrices!
   }
   
   // Good: matrices are cached in camera automatically
   camera.update(pos, angles);  // Updates matrices once
   for (auto& pos : positions) {
       auto screen = camera.world_to_screen(pos);  // Uses cached matrices
   }
   ```

---

## Getting More Help

If your issue isn't covered here:

1. **Check the docs**: [API Overview](api_overview.md), [Tutorials](tutorials.md)
2. **Search GitHub issues**: [Issues page](https://github.com/orange-cpp/omath/issues)
3. **Ask on Discord**: [Join community](https://discord.gg/eDgdaWbqwZ)
4. **Open a new issue**: Include:
   - OMath version
   - Compiler and version
   - Minimal reproducible example
   - What you expected vs what happened

---

*Last updated: 1 Nov 2025*
