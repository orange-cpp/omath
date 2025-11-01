# Tutorials

This page provides step-by-step tutorials for common OMath use cases.

---

## Tutorial 1: Basic Vector Math

Learn the fundamentals of vector operations in OMath.

### Step 1: Include OMath

```cpp
#include <omath/omath.hpp>
#include <iostream>

using namespace omath;
```

### Step 2: Create Vectors

```cpp
// 2D vectors
Vector2<float> v2a{3.0f, 4.0f};
Vector2<float> v2b{1.0f, 2.0f};

// 3D vectors
Vector3<float> v3a{1.0f, 2.0f, 3.0f};
Vector3<float> v3b{4.0f, 5.0f, 6.0f};

// 4D vectors (often used for homogeneous coordinates)
Vector4<float> v4{1.0f, 2.0f, 3.0f, 1.0f};
```

### Step 3: Perform Operations

```cpp
// Addition
auto sum = v3a + v3b;  // {5, 7, 9}

// Subtraction
auto diff = v3a - v3b;  // {-3, -3, -3}

// Scalar multiplication
auto scaled = v3a * 2.0f;  // {2, 4, 6}

// Dot product
float dot = v3a.dot(v3b);  // 32.0

// Cross product (3D only)
auto cross = v3a.cross(v3b);  // {-3, 6, -3}

// Length
float len = v3a.length();  // ~3.74

// Normalization (safe - returns original if length is zero)
auto normalized = v3a.normalized();

// Distance between vectors
float dist = v3a.distance_to(v3b);  // ~5.196
```

### Step 4: Angle Calculations

```cpp
if (auto angle = v3a.angle_between(v3b)) {
    std::cout << "Angle in degrees: " << angle->as_degrees() << "\n";
    std::cout << "Angle in radians: " << angle->as_radians() << "\n";
} else {
    std::cout << "Cannot compute angle (zero-length vector)\n";
}

// Check if perpendicular
if (v3a.is_perpendicular(v3b)) {
    std::cout << "Vectors are perpendicular\n";
}
```

**Key takeaways:**
- All vector operations are type-safe and constexpr-friendly
- Safe normalization never produces NaN
- Angle calculations use `std::expected` for error handling

---

## Tutorial 2: World-to-Screen Projection

Project 3D coordinates to 2D screen space for overlays and ESP.

### Step 1: Choose Your Game Engine

```cpp
#include <omath/omath.hpp>

// For Source Engine games (CS:GO, TF2, etc.)
using namespace omath::source_engine;

// Or for other engines:
// using namespace omath::unity_engine;
// using namespace omath::unreal_engine;
// using namespace omath::frostbite_engine;
```

### Step 2: Set Up the Camera

```cpp
using namespace omath;
using namespace omath::projection;

// Define viewport (screen dimensions)
ViewPort viewport{1920.0f, 1080.0f};

// Define field of view
auto fov = FieldOfView::from_degrees(90.0f);

// Camera position and angles
Vector3<float> camera_pos{0.0f, 0.0f, 100.0f};
ViewAngles camera_angles{
    PitchAngle::from_degrees(0.0f),
    YawAngle::from_degrees(0.0f),
    RollAngle::from_degrees(0.0f)
};

// Create camera (using Source Engine in this example)
Camera camera(
    camera_pos,
    camera_angles,
    viewport,
    fov,
    0.1f,    // near plane
    1000.0f  // far plane
);
```

### Step 3: Project 3D Points

```cpp
// 3D world position (e.g., enemy player position)
Vector3<float> enemy_pos{150.0f, 200.0f, 75.0f};

// Project to screen
if (auto screen = camera.world_to_screen(enemy_pos)) {
    std::cout << "Enemy on screen at: "
              << screen->x << ", " << screen->y << "\n";
    
    // Draw ESP box or marker at screen->x, screen->y
    // Note: screen coordinates are in viewport space (0-width, 0-height)
} else {
    // Enemy is not visible (behind camera or outside frustum)
    std::cout << "Enemy not visible\n";
}
```

### Step 4: Update Camera for Each Frame

```cpp
void render_frame() {
    // Read current camera data from game
    Vector3<float> new_pos = read_camera_position();
    ViewAngles new_angles = read_camera_angles();
    
    // Update camera
    camera.update(new_pos, new_angles);
    
    // Project all entities
    for (const auto& entity : entities) {
        if (auto screen = camera.world_to_screen(entity.position)) {
            draw_esp_box(screen->x, screen->y);
        }
    }
}
```

**Key takeaways:**
- Choose the engine trait that matches your target game
- `world_to_screen()` returns `std::optional` - always check the result
- Update camera each frame for accurate projections
- Screen coordinates are in the viewport space you defined

---

## Tutorial 3: Projectile Prediction (Aim-Bot)

Calculate where to aim to hit a moving target.

### Step 1: Define Projectile Properties

```cpp
#include <omath/omath.hpp>
#include <omath/projectile_prediction/proj_pred_engine_legacy.hpp>

using namespace omath;
using namespace omath::projectile_prediction;

// Define your weapon's projectile
Projectile bullet;
bullet.origin = Vector3<float>{0, 0, 0};  // Shooter position
bullet.speed = 800.0f;  // Muzzle velocity (m/s or game units/s)
bullet.gravity = Vector3<float>{0, 0, -9.81f};  // Gravity vector
```

### Step 2: Define Target State

```cpp
// Target information (enemy player)
Target enemy;
enemy.position = Vector3<float>{100, 200, 50};  // Current position
enemy.velocity = Vector3<float>{10, 5, 0};      // Current velocity
```

### Step 3: Calculate Aim Point

```cpp
// Create prediction engine
// Use AVX2 version if available for better performance:
// ProjPredEngineAVX2 engine;
ProjPredEngineLegacy engine;

// Calculate where to aim
if (auto aim_point = engine.maybe_calculate_aim_point(bullet, enemy)) {
    std::cout << "Aim at: "
              << aim_point->x << ", "
              << aim_point->y << ", "
              << aim_point->z << "\n";
    
    // Calculate angles to aim_point
    Vector3<float> aim_direction = (*aim_point - bullet.origin).normalized();
    
    // Convert to view angles (engine-specific)
    // ViewAngles angles = calculate_angles_to_direction(aim_direction);
    // set_aim_angles(angles);
} else {
    // Cannot hit target (too fast, out of range, etc.)
    std::cout << "Target cannot be hit\n";
}
```

### Step 4: Handle Different Scenarios

```cpp
// Stationary target
Target stationary;
stationary.position = Vector3<float>{100, 100, 100};
stationary.velocity = Vector3<float>{0, 0, 0};
// aim_point will equal position for stationary targets

// Fast-moving target
Target fast;
fast.position = Vector3<float>{100, 100, 100};
fast.velocity = Vector3<float>{50, 0, 0};  // Moving very fast
// May return nullopt if target is too fast

// Target at different heights
Target aerial;
aerial.position = Vector3<float>{100, 100, 200};  // High up
aerial.velocity = Vector3<float>{5, 5, -10};      // Falling
// Gravity will be factored into the calculation
```

### Step 5: Performance Optimization

```cpp
// For better performance on modern CPUs, use AVX2:
#include <omath/projectile_prediction/proj_pred_engine_avx2.hpp>

ProjPredEngineAVX2 fast_engine;  // 2-4x faster than legacy

// Use the same way as legacy engine
if (auto aim = fast_engine.maybe_calculate_aim_point(bullet, enemy)) {
    // Process aim point
}
```

**Key takeaways:**
- Always check if aim point exists before using
- Velocity must be in same units as position/speed
- Gravity vector points down (typically negative Z or Y depending on engine)
- Use AVX2 engine when possible for better performance
- Returns `nullopt` when target is unreachable

---

## Tutorial 4: Collision Detection

Perform ray-casting and intersection tests.

### Step 1: Ray-Plane Intersection

```cpp
#include <omath/omath.hpp>

using namespace omath;

// Define a ground plane (Z=0, normal pointing up)
Plane ground{
    Vector3<float>{0, 0, 0},  // Point on plane
    Vector3<float>{0, 0, 1}   // Normal vector (Z-up)
};

// Define a ray (e.g., looking downward from above)
Vector3<float> ray_origin{10, 20, 100};
Vector3<float> ray_direction{0, 0, -1};  // Pointing down

// Test intersection
if (auto hit = ground.intersects_ray(ray_origin, ray_direction)) {
    std::cout << "Hit ground at: "
              << hit->x << ", " << hit->y << ", " << hit->z << "\n";
    // Expected: (10, 20, 0)
} else {
    std::cout << "Ray does not intersect plane\n";
}
```

### Step 2: Distance to Plane

```cpp
// Calculate signed distance from point to plane
Vector3<float> point{10, 20, 50};
float distance = ground.distance_to_point(point);

std::cout << "Distance to ground: " << distance << "\n";
// Expected: 50.0 (50 units above ground)

// Negative distance means point is below the plane
Vector3<float> below{10, 20, -5};
float dist_below = ground.distance_to_point(below);
// Expected: -5.0
```

### Step 3: Axis-Aligned Bounding Box

```cpp
#include <omath/3d_primitives/box.hpp>

// Create a bounding box
Box bbox{
    Vector3<float>{0, 0, 0},     // Min corner
    Vector3<float>{100, 100, 100} // Max corner
};

// Test if point is inside
Vector3<float> inside{50, 50, 50};
if (bbox.contains(inside)) {
    std::cout << "Point is inside box\n";
}

Vector3<float> outside{150, 50, 50};
if (!bbox.contains(outside)) {
    std::cout << "Point is outside box\n";
}

// Box-box intersection
Box other{
    Vector3<float>{50, 50, 50},
    Vector3<float>{150, 150, 150}
};

if (bbox.intersects(other)) {
    std::cout << "Boxes overlap\n";
}
```

### Step 4: Line Tracing

```cpp
#include <omath/collision/line_tracer.hpp>

using namespace omath::collision;

// Ray-triangle intersection
Vector3<float> v0{0, 0, 0};
Vector3<float> v1{100, 0, 0};
Vector3<float> v2{0, 100, 0};

Vector3<float> ray_start{25, 25, 100};
Vector3<float> ray_dir{0, 0, -1};

LineTracer tracer;
if (auto hit = tracer.ray_triangle_intersect(ray_start, ray_dir, v0, v1, v2)) {
    std::cout << "Hit triangle at: "
              << hit->point.x << ", "
              << hit->point.y << ", "
              << hit->point.z << "\n";
    std::cout << "Hit distance: " << hit->distance << "\n";
    std::cout << "Surface normal: "
              << hit->normal.x << ", "
              << hit->normal.y << ", "
              << hit->normal.z << "\n";
}
```

**Key takeaways:**
- Plane normals should be unit vectors
- Ray direction should typically be normalized
- Signed distance indicates which side of plane a point is on
- AABB tests are very fast for broad-phase collision detection
- Line tracer provides hit point, distance, and surface normal

---

## Tutorial 5: Pattern Scanning

Search for byte patterns in memory.

### Step 1: Basic Pattern Scanning

```cpp
#include <omath/utility/pattern_scan.hpp>
#include <vector>

using namespace omath;

// Memory to search (e.g., from a loaded module)
std::vector<uint8_t> memory = {
    0x48, 0x8B, 0x05, 0xAA, 0xBB, 0xCC, 0xDD,
    0x48, 0x85, 0xC0, 0x74, 0x10,
    // ... more bytes
};

// Pattern with wildcards (?? = match any byte)
PatternView pattern{"48 8B 05 ?? ?? ?? ?? 48 85 C0"};

// Scan for pattern
if (auto result = pattern_scan(memory, pattern)) {
    std::cout << "Pattern found at offset: " << result->offset << "\n";
    
    // Extract wildcard values if needed
    // result->wildcards contains the matched bytes at ?? positions
} else {
    std::cout << "Pattern not found\n";
}
```

### Step 2: PE File Scanning

```cpp
#include <omath/utility/pe_pattern_scan.hpp>

// Scan a PE file (EXE or DLL)
PEPatternScanner scanner("game.exe");

PatternView pattern{"E8 ?? ?? ?? ?? 85 C0 75 ??"};

if (auto rva = scanner.scan_pattern(pattern)) {
    std::cout << "Pattern found at RVA: 0x"
              << std::hex << *rva << std::dec << "\n";
    
    // Convert RVA to absolute address if needed
    uintptr_t base_address = get_module_base("game.exe");
    uintptr_t absolute = base_address + *rva;
} else {
    std::cout << "Pattern not found in PE file\n";
}
```

### Step 3: Multiple Patterns

```cpp
// Search for multiple patterns
std::vector<PatternView> patterns{
    PatternView{"48 8B 05 ?? ?? ?? ??"},
    PatternView{"E8 ?? ?? ?? ?? 85 C0"},
    PatternView{"FF 15 ?? ?? ?? ?? 48 8B"}
};

for (size_t i = 0; i < patterns.size(); ++i) {
    if (auto result = pattern_scan(memory, patterns[i])) {
        std::cout << "Pattern " << i << " found at: "
                  << result->offset << "\n";
    }
}
```

### Step 4: Pattern with Masks

```cpp
// Alternative: use mask-based patterns
// Pattern: bytes to match
std::vector<uint8_t> pattern_bytes{0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00};

// Mask: 'x' = must match, '?' = wildcard
std::string mask{"xxx????"};

// Custom scan function
auto scan_with_mask = [&](const std::vector<uint8_t>& data) {
    for (size_t i = 0; i < data.size() - pattern_bytes.size(); ++i) {
        bool match = true;
        for (size_t j = 0; j < pattern_bytes.size(); ++j) {
            if (mask[j] == 'x' && data[i + j] != pattern_bytes[j]) {
                match = false;
                break;
            }
        }
        if (match) return i;
    }
    return size_t(-1);
};
```

**Key takeaways:**
- Use `??` in pattern strings for wildcards
- PE scanner works with files and modules
- Pattern scanning is useful for finding functions, vtables, or data
- Always validate found addresses before use
- Patterns may have multiple matches - consider context

---

## Tutorial 6: Angles and View Angles

Work with game camera angles properly.

### Step 1: Understanding Angle Types

```cpp
#include <omath/omath.hpp>

using namespace omath;

// Generic angle with custom range
auto angle1 = Angle<float, 0.0f, 360.0f>::from_degrees(45.0f);
auto angle2 = Angle<float, -180.0f, 180.0f>::from_degrees(270.0f);

// Specialized camera angles
auto pitch = PitchAngle::from_degrees(-10.0f);  // Looking down
auto yaw = YawAngle::from_degrees(90.0f);       // Looking right
auto roll = RollAngle::from_degrees(0.0f);      // No tilt
```

### Step 2: Angle Conversions

```cpp
// Create from degrees
auto deg_angle = PitchAngle::from_degrees(45.0f);

// Get as radians
float radians = deg_angle.as_radians();
std::cout << "45° = " << radians << " radians\n";

// Get as degrees
float degrees = deg_angle.as_degrees();
std::cout << "Value: " << degrees << "°\n";
```

### Step 3: View Angles (Camera)

```cpp
// Pitch: vertical rotation (-89° to 89°)
// Yaw: horizontal rotation (-180° to 180°)
// Roll: camera tilt (-180° to 180°)

ViewAngles camera_angles{
    PitchAngle::from_degrees(-15.0f),  // Looking slightly down
    YawAngle::from_degrees(45.0f),     // Facing northeast
    RollAngle::from_degrees(0.0f)      // No tilt
};

// Access individual components
float pitch_val = camera_angles.pitch.as_degrees();
float yaw_val = camera_angles.yaw.as_degrees();
float roll_val = camera_angles.roll.as_degrees();
```

### Step 4: Calculating Look-At Angles

```cpp
using namespace omath::source_engine;  // Or your game's engine

Vector3<float> camera_pos{0, 0, 100};
Vector3<float> target_pos{100, 100, 100};

// Calculate angles to look at target
ViewAngles look_at = CameraTrait::calc_look_at_angle(camera_pos, target_pos);

std::cout << "Pitch: " << look_at.pitch.as_degrees() << "°\n";
std::cout << "Yaw: " << look_at.yaw.as_degrees() << "°\n";
std::cout << "Roll: " << look_at.roll.as_degrees() << "°\n";
```

### Step 5: Angle Arithmetic

```cpp
// Angles support arithmetic with automatic normalization
auto angle1 = YawAngle::from_degrees(170.0f);
auto angle2 = YawAngle::from_degrees(20.0f);

// Addition (wraps around)
auto sum = angle1 + angle2;  // 190° → normalized to -170°

// Subtraction
auto diff = angle2 - angle1;  // -150°

// Scaling
auto scaled = angle1 * 2.0f;
```

**Key takeaways:**
- Use specialized angle types for camera angles (PitchAngle, YawAngle, RollAngle)
- Angles automatically normalize to their valid ranges
- Each game engine may have different angle conventions
- Use engine traits to calculate look-at angles correctly

---

## Next Steps

Now that you've completed these tutorials, explore:

- **[API Overview](api_overview.md)** - Complete API reference
- **[Engine Documentation](engines/)** - Engine-specific features
- **[Examples](../examples/)** - More code examples
- **[Getting Started](getting_started.md)** - Quick start guide

---

*Last updated: 1 Nov 2025*
