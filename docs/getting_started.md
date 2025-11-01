# Getting Started with OMath

Welcome to OMath! This guide will help you get up and running with the library quickly.

## What is OMath?

OMath is a modern, blazingly fast C++ math library designed for:
- **Game development** and cheat development
- **Graphics programming** (DirectX/OpenGL/Vulkan)
- **3D applications** with support for multiple game engines
- **High-performance computing** with AVX2 optimizations

Key features:
- 100% independent, no legacy C++ code
- Fully `constexpr` template-based design
- Zero additional dependencies (except for unit tests)
- Cross-platform (Windows, macOS, Linux)
- Built-in support for Source, Unity, Unreal, Frostbite, IWEngine, and OpenGL coordinate systems

---

## Installation

Choose one of the following methods to install OMath:

### Using vcpkg (Recommended)

```bash
vcpkg install orange-math
```

Then in your CMakeLists.txt:
```cmake
find_package(omath CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE omath::omath)
```

### Using xrepo

```bash
xrepo install omath
```

Then in your xmake.lua:
```lua
add_requires("omath")
target("your_target")
    add_packages("omath")
```

### Building from Source

See the detailed [Installation Guide](install.md) for complete instructions.

---

## Quick Example

Here's a simple example to get you started:

```cpp
#include <omath/omath.hpp>
#include <iostream>

int main() {
    using namespace omath;
    
    // Create 3D vectors
    Vector3<float> a{1.0f, 2.0f, 3.0f};
    Vector3<float> b{4.0f, 5.0f, 6.0f};
    
    // Vector operations
    auto sum = a + b;                    // Vector addition
    auto dot_product = a.dot(b);         // Dot product: 32.0
    auto cross_product = a.cross(b);     // Cross product: (-3, 6, -3)
    auto length = a.length();            // Length: ~3.74
    auto normalized = a.normalized();    // Unit vector
    
    std::cout << "Sum: [" << sum.x << ", " << sum.y << ", " << sum.z << "]\n";
    std::cout << "Dot product: " << dot_product << "\n";
    std::cout << "Length: " << length << "\n";
    
    return 0;
}
```

---

## Core Concepts

### 1. Vectors

OMath provides 2D, 3D, and 4D vector types:

```cpp
using namespace omath;

Vector2<float> vec2{1.0f, 2.0f};
Vector3<float> vec3{1.0f, 2.0f, 3.0f};
Vector4<float> vec4{1.0f, 2.0f, 3.0f, 4.0f};
```

All vector types support:
- Arithmetic operations (+, -, *, /)
- Dot and cross products (where applicable)
- Length and distance calculations
- Normalization
- Component-wise operations

See: [Vector2](linear_algebra/vector2.md), [Vector3](linear_algebra/vector3.md), [Vector4](linear_algebra/vector4.md)

### 2. Matrices

4x4 matrices for transformations:

```cpp
using namespace omath;

Mat4X4 matrix = Mat4X4::identity();
// Use for transformations, projections, etc.
```

See: [Matrix Documentation](linear_algebra/mat.md)

### 3. Angles

Strong-typed angle system with automatic range management:

```cpp
using namespace omath;

auto angle = Angle<float, 0.0f, 360.0f>::from_degrees(45.0f);
auto radians = angle.as_radians();

// View angles for camera systems
ViewAngles view{
    PitchAngle::from_degrees(-10.0f),
    YawAngle::from_degrees(90.0f),
    RollAngle::from_degrees(0.0f)
};
```

See: [Angle](trigonometry/angle.md), [View Angles](trigonometry/view_angles.md)

### 4. 3D Projection

Built-in camera and projection systems:

```cpp
using namespace omath;
using namespace omath::projection;

ViewPort viewport{1920.0f, 1080.0f};
auto fov = FieldOfView::from_degrees(90.0f);

// Example using Source Engine
using namespace omath::source_engine;
Camera cam(
    Vector3<float>{0, 0, 100},  // Position
    ViewAngles{},                // Angles
    viewport,
    fov,
    0.1f,   // near plane
    1000.0f // far plane
);

// Project 3D point to 2D screen
Vector3<float> world_pos{100, 50, 75};
if (auto screen_pos = cam.world_to_screen(world_pos)) {
    std::cout << "Screen: " << screen_pos->x << ", " << screen_pos->y << "\n";
}
```

See: [Camera](projection/camera.md)

### 5. Game Engine Support

OMath provides pre-configured traits for major game engines:

```cpp
// Source Engine
#include <omath/engines/source_engine/camera.hpp>
using SourceCamera = omath::source_engine::Camera;

// Unity Engine
#include <omath/engines/unity_engine/camera.hpp>
using UnityCamera = omath::unity_engine::Camera;

// Unreal Engine
#include <omath/engines/unreal_engine/camera.hpp>
using UnrealCamera = omath::unreal_engine::Camera;

// And more: OpenGL, Frostbite, IWEngine
```

Each engine has its own coordinate system conventions automatically handled.

See: Engine-specific docs in [engines/](engines/) folder

---

## Common Use Cases

### World-to-Screen Projection

```cpp
using namespace omath;
using namespace omath::source_engine;

Camera cam = /* initialize camera */;
Vector3<float> enemy_position{100, 200, 50};

if (auto screen = cam.world_to_screen(enemy_position)) {
    // Draw ESP box at screen->x, screen->y
    std::cout << "Enemy on screen at: " << screen->x << ", " << screen->y << "\n";
} else {
    // Enemy not visible (behind camera or outside frustum)
}
```

### Projectile Prediction

```cpp
using namespace omath::projectile_prediction;

Projectile bullet{
    Vector3<float>{0, 0, 0},      // shooter position
    1000.0f,                       // muzzle velocity (m/s)
    Vector3<float>{0, 0, -9.81f}  // gravity
};

Target enemy{
    Vector3<float>{100, 200, 50},  // position
    Vector3<float>{10, 0, 0}       // velocity
};

// Calculate where to aim
ProjPredEngineLegacy engine;
if (auto aim_point = engine.maybe_calculate_aim_point(bullet, enemy)) {
    // Aim at *aim_point to hit moving target
}
```

See: [Projectile Prediction](projectile_prediction/projectile_engine.md)

### Collision Detection

```cpp
using namespace omath;

// Ray-plane intersection
Plane ground{
    Vector3<float>{0, 0, 0},  // point on plane
    Vector3<float>{0, 0, 1}   // normal (pointing up)
};

Vector3<float> ray_origin{0, 0, 100};
Vector3<float> ray_direction{0, 0, -1};

if (auto hit = ground.intersects_ray(ray_origin, ray_direction)) {
    std::cout << "Hit ground at: " << hit->x << ", " << hit->y << ", " << hit->z << "\n";
}
```

See: [Collision Detection](collision/line_tracer.md)

### Pattern Scanning

```cpp
#include <omath/utility/pattern_scan.hpp>

using namespace omath;

std::vector<uint8_t> memory = /* ... */;
PatternView pattern{"48 8B 05 ?? ?? ?? ?? 48 85 C0"};

if (auto result = pattern_scan(memory, pattern)) {
    std::cout << "Pattern found at offset: " << result->offset << "\n";
}
```

See: [Pattern Scanning](utility/pattern_scan.md)

---

## Next Steps

Now that you have the basics, explore these topics:

1. **[API Reference](index.md)** - Complete API documentation
2. **[Examples](../examples/)** - Working code examples
3. **[Engine-Specific Features](engines/)** - Deep dive into game engine support
4. **[Advanced Topics](#)** - Performance optimization, custom traits, etc.

---

## Getting Help

- **Documentation**: [http://libomath.org](http://libomath.org)
- **Discord**: [Join our community](https://discord.gg/eDgdaWbqwZ)
- **Telegram**: [@orangennotes](https://t.me/orangennotes)
- **Issues**: [GitHub Issues](https://github.com/orange-cpp/omath/issues)

---

*Last updated: 1 Nov 2025*
