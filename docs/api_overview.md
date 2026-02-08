# API Overview

This document provides a high-level overview of OMath's API, organized by functionality area.

---

## Module Organization

OMath is organized into several logical modules:

### Core Mathematics
- **Linear Algebra** - Vectors, matrices, triangles
- **Trigonometry** - Angles, view angles, trigonometric functions
- **3D Primitives** - Boxes, planes, meshes, geometric shapes

### Game Development
- **Collision Detection** - Ray tracing, GJK/EPA algorithms, mesh collision, intersection tests
- **Projectile Prediction** - Ballistics and aim-assist calculations
- **Projection** - Camera systems and world-to-screen transformations
- **Pathfinding** - A* algorithm, navigation meshes

### Engine Support
- **Source Engine** - Valve's Source Engine (CS:GO, TF2, etc.)
- **Unity Engine** - Unity game engine
- **Unreal Engine** - Epic's Unreal Engine
- **Frostbite Engine** - EA's Frostbite Engine
- **IW Engine** - Infinity Ward's engine (Call of Duty)
- **OpenGL Engine** - Canonical OpenGL coordinate system

### Utilities
- **Color** - RGBA color representation
- **Pattern Scanning** - Memory pattern search (wildcards, PE files)
- **Reverse Engineering** - Internal/external memory manipulation

---

## Core Types

### Vectors

All vector types are template-based and support arithmetic types.

| Type | Description | Key Methods |
|------|-------------|-------------|
| `Vector2<T>` | 2D vector | `length()`, `normalized()`, `dot()`, `distance_to()` |
| `Vector3<T>` | 3D vector | `length()`, `normalized()`, `dot()`, `cross()`, `angle_between()` |
| `Vector4<T>` | 4D vector | Extends Vector3 with `w` component |

**Common aliases:**
```cpp
using Vec2f = Vector2<float>;
using Vec3f = Vector3<float>;
using Vec4f = Vector4<float>;
```

**Key features:**
- Component-wise arithmetic (+, -, *, /)
- Scalar multiplication/division
- Dot and cross products
- Safe normalization (returns original if length is zero)
- Distance calculations
- Angle calculations with error handling
- Hash support for `float` variants
- `std::formatter` support

### Matrices

| Type | Description | Key Methods |
|------|-------------|-------------|
| `Mat4X4` | 4×4 matrix | `identity()`, `transpose()`, `determinant()`, `inverse()` |

**Use cases:**
- Transformation matrices
- View matrices
- Projection matrices
- Model-view-projection pipelines

### Angles

Strong-typed angle system with compile-time range enforcement:

| Type | Range | Description |
|------|-------|-------------|
| `Angle<T, Min, Max, Flags>` | Custom | Generic angle type with bounds |
| `PitchAngle` | [-89°, 89°] | Vertical camera rotation |
| `YawAngle` | [-180°, 180°] | Horizontal camera rotation |
| `RollAngle` | [-180°, 180°] | Camera roll |
| `ViewAngles` | - | Composite pitch/yaw/roll |

**Features:**
- Automatic normalization/clamping based on flags
- Conversions between degrees and radians
- Type-safe arithmetic
- Prevents common angle bugs

---

## Projection System

### Camera

Generic camera template that works with any engine trait:

```cpp
template<class MatrixType, class AnglesType, class EngineTrait>
class Camera;
```

**Engine-specific cameras:**
```cpp
omath::source_engine::Camera      // Source Engine
omath::unity_engine::Camera       // Unity
omath::unreal_engine::Camera      // Unreal
omath::frostbite_engine::Camera   // Frostbite
omath::iw_engine::Camera          // IW Engine
omath::opengl_engine::Camera      // OpenGL
```

**Core methods:**
- `world_to_screen(Vector3<float>)` - Project 3D point to 2D screen
- `get_view_matrix()` - Get current view matrix
- `get_projection_matrix()` - Get current projection matrix
- `update(position, angles)` - Update camera state

**Supporting types:**
- `ViewPort` - Screen dimensions and aspect ratio
- `FieldOfView` - FOV in degrees with validation
- `ProjectionError` - Error codes for projection failures

---

## Collision Detection

### GJK/EPA Algorithms

Advanced convex shape collision detection using the Gilbert-Johnson-Keerthi and Expanding Polytope algorithms:

```cpp
namespace omath::collision {
    template<class ColliderType>
    class GjkAlgorithm;
    
    template<class ColliderType>
    class Epa;
}
```

**GJK (Gilbert-Johnson-Keerthi):**
* Detects collision between two convex shapes
* Returns a 4-point simplex when collision is detected
* O(k) complexity where k is typically < 20 iterations
* Works with any collider implementing `find_abs_furthest_vertex()`

**EPA (Expanding Polytope Algorithm):**
* Computes penetration depth and separation normal
* Takes GJK's output simplex as input
* Provides contact information for physics simulation
* Configurable iteration limit and convergence tolerance

**Supporting Types:**

| Type | Description | Key Features |
|------|-------------|--------------|
| `Simplex<VectorType>` | 1-4 point geometric simplex | Fixed capacity, GJK iteration support |
| `MeshCollider<MeshType>` | Convex mesh collider | Support function for GJK/EPA |
| `GjkHitInfo<VertexType>` | Collision result | Hit flag and simplex |
| `Epa::Result` | Penetration info | Depth, normal, iteration count |

### LineTracer

Ray-casting and line tracing utilities:

```cpp
namespace omath::collision {
    class LineTracer;
}
```

**Features:**
- Ray-triangle intersection (Möller-Trumbore algorithm)
- Ray-plane intersection
- Ray-box intersection
- Distance calculations
- Normal calculations at hit points

### 3D Primitives

| Type | Description | Key Methods |
|------|-------------|-------------|
| `Plane` | Infinite plane | `intersects_ray()`, `distance_to_point()` |
| `Box` | Axis-aligned bounding box | `contains()`, `intersects()` |
| `Mesh` | Polygonal mesh with transforms | `vertex_to_world_space()`, `make_face_in_world_space()` |

**Mesh Features:**
* Vertex buffer (VBO) and index buffer (VAO/EBO) storage
* Position, rotation, and scale transformations
* Cached transformation matrix
* Engine-specific coordinate system support
* Compatible with `MeshCollider` for collision detection

---

## Projectile Prediction

### Interfaces

**`ProjPredEngineInterface`** - Base interface for all prediction engines

```cpp
virtual std::optional<Vector3<float>>
maybe_calculate_aim_point(const Projectile&, const Target&) const = 0;
```

### Implementations

| Engine | Description | Optimizations |
|--------|-------------|---------------|
| `ProjPredEngineLegacy` | Standard implementation | Portable, works everywhere |
| `ProjPredEngineAVX2` | AVX2 optimized | 2-4x faster on modern CPUs |

### Supporting Types

**`Projectile`** - Defines projectile properties:
```cpp
struct Projectile {
    Vector3<float> origin;
    float speed;
    Vector3<float> gravity;
    // ... additional properties
};
```

**`Target`** - Defines target state:
```cpp
struct Target {
    Vector3<float> position;
    Vector3<float> velocity;
    // ... additional properties
};
```

---

## Pathfinding

### A* Algorithm

```cpp
namespace omath::pathfinding {
    template<typename NodeType>
    class AStar;
}
```

**Features:**
- Generic node type support
- Customizable heuristics
- Efficient priority queue implementation
- Path reconstruction

### Navigation Mesh

```cpp
namespace omath::pathfinding {
    class NavigationMesh;
}
```

**Features:**
- Triangle-based navigation
- Neighbor connectivity
- Walkable area definitions

---

## Engine Traits

Each game engine has a trait system providing engine-specific math:

### CameraTrait

Implements camera math for an engine:
- `calc_look_at_angle()` - Calculate angles to look at a point
- `calc_view_matrix()` - Build view matrix from angles and position
- `calc_projection_matrix()` - Build projection matrix from FOV and viewport

### MeshTrait

Provides mesh transformation for an engine:
- `rotation_matrix()` - Build rotation matrix from engine-specific angles
- Handles coordinate system differences (Y-up vs Z-up, left/right-handed)
- Used by `Mesh` class for local-to-world transformations

### PredEngineTrait

Provides physics/ballistics specific to an engine:
- Gravity vectors
- Coordinate system conventions
- Unit conversions
- Physics parameters

### Available Traits

| Engine | Camera Trait | Mesh Trait | Pred Engine Trait | Constants | Formulas |
|--------|--------------|------------|-------------------|-----------|----------|
| Source Engine | ✓ | ✓ | ✓ | ✓ | ✓ |
| Unity Engine | ✓ | ✓ | ✓ | ✓ | ✓ |
| Unreal Engine | ✓ | ✓ | ✓ | ✓ | ✓ |
| Frostbite | ✓ | ✓ | ✓ | ✓ | ✓ |
| IW Engine | ✓ | ✓ | ✓ | ✓ | ✓ |
| OpenGL | ✓ | ✓ | ✓ | ✓ | ✓ |

**Documentation:**
- See `docs/engines/<engine_name>/` for detailed per-engine docs
- Each engine has separate docs for camera_trait, mesh_trait, pred_engine_trait, constants, and formulas

---

## Utility Functions

### Color

```cpp
struct Color {
    uint8_t r, g, b, a;
    
    // Conversions
    static Color from_hsv(float h, float s, float v);
    static Color from_hex(uint32_t hex);
    uint32_t to_hex() const;
    
    // Blending
    Color blend(const Color& other, float t) const;
};
```

### Pattern Scanning

**Binary pattern search with wildcards:**

```cpp
// Pattern with wildcards (?? = any byte)
PatternView pattern{"48 8B 05 ?? ?? ?? ?? 48 85 C0"};

// Scan memory
auto result = pattern_scan(memory_buffer, pattern);
if (result) {
    std::cout << "Found at offset: " << result->offset << "\n";
}
```

**PE file scanning:**

```cpp
PEPatternScanner scanner("target.exe");
if (auto addr = scanner.scan_pattern(pattern)) {
    std::cout << "Found at RVA: " << *addr << "\n";
}
```

### Reverse Engineering

**External memory access:**
```cpp
ExternalRevObject process("game.exe");
Vector3<float> position = process.read<Vector3<float>>(address);
process.write(address, new_position);
```

**Internal memory access:**
```cpp
InternalRevObject memory;
auto value = memory.read<float>(address);
memory.write(address, new_value);
```

---

## Concepts and Constraints

OMath uses C++20 concepts for type safety:

```cpp
template<class T>
concept Arithmetic = std::is_arithmetic_v<T>;

template<class EngineTrait>
concept CameraEngineConcept = requires(EngineTrait t) {
    { t.calc_look_at_angle(...) } -> /* returns angles */;
    { t.calc_view_matrix(...) } -> /* returns matrix */;
    { t.calc_projection_matrix(...) } -> /* returns matrix */;
};
```

---

## Error Handling

OMath uses modern C++ error handling:

### std::expected (C++23)

```cpp
std::expected<Angle<...>, Vector3Error>
angle_between(const Vector3& other) const;

if (auto angle = v1.angle_between(v2)) {
    // Success: use *angle
} else {
    // Error: angle.error() gives Vector3Error
}
```

### std::optional

```cpp
std::optional<Vector2<float>>
world_to_screen(const Vector3<float>& world);

if (auto screen = camera.world_to_screen(pos)) {
    // Success: use screen->x, screen->y
} else {
    // Point not visible
}
```

### Error Codes

```cpp
enum class ProjectionError {
    SUCCESS = 0,
    POINT_BEHIND_CAMERA,
    INVALID_VIEWPORT,
    // ...
};
```

---

## Performance Considerations

### constexpr Support

Most operations are `constexpr` where possible:

```cpp
constexpr Vector3<float> v{1, 2, 3};
constexpr auto len_sq = v.length_sqr();  // Computed at compile time
```

### AVX2 Optimizations

Use AVX2 variants when available:

```cpp
// Standard: portable but slower
ProjPredEngineLegacy legacy_engine;

// AVX2: 2-4x faster on modern CPUs
ProjPredEngineAVX2 fast_engine;
```

**When to use AVX2:**
- Modern Intel/AMD processors (2013+)
- Performance-critical paths
- Batch operations

**When to use Legacy:**
- Older processors
- ARM platforms
- Guaranteed compatibility

### Cache Efficiency

```cpp
// Good: contiguous storage
std::vector<Vector3<float>> positions;

// Good: structure of arrays for SIMD
struct Particles {
    std::vector<float> x, y, z;
};
```

---

## Platform Support

| Platform | Support | Notes |
|----------|---------|-------|
| Windows | ✓ | MSVC, Clang, GCC |
| Linux | ✓ | GCC, Clang |
| macOS | ✓ | Clang |

**Minimum requirements:**
- C++20 compiler
- C++23 recommended for `std::expected`

---

## Thread Safety

- **Vector/Matrix types**: Thread-safe (immutable operations)
- **Camera**: Not thread-safe (mutable state)
- **Pattern scanning**: Thread-safe (read-only operations)
- **Memory access**: Depends on OS/process synchronization

**Thread-safe example:**
```cpp
// Safe: each thread gets its own camera
std::vector<std::thread> threads;
for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([i]() {
        Camera camera = /* create camera */;
        // Use camera in this thread
    });
}
```

---

## Best Practices

### 1. Use Type Aliases

```cpp
using Vec3f = omath::Vector3<float>;
using Mat4 = omath::Mat4X4;
```

### 2. Prefer constexpr When Possible

```cpp
constexpr auto compute_at_compile_time() {
    Vector3<float> v{1, 2, 3};
    return v.length_sqr();
}
```

### 3. Check Optional/Expected Results

```cpp
// Good
if (auto result = camera.world_to_screen(pos)) {
    use(*result);
}

// Bad - may crash
auto result = camera.world_to_screen(pos);
use(result->x);  // Undefined behavior if nullopt
```

### 4. Use Engine-Specific Types

```cpp
// Good: uses correct coordinate system
using namespace omath::source_engine;
Camera camera = /* ... */;

// Bad: mixing engine types
using UnityCamera = omath::unity_engine::Camera;
using SourceAngles = omath::source_engine::ViewAngles;
UnityCamera camera{pos, SourceAngles{}}; // Wrong!
```

---

## See Also

- [Getting Started Guide](getting_started.md)
- [Installation Instructions](install.md)
- [Examples Directory](examples/index.md)
- Individual module documentation in respective folders

---

*Last updated: 13 Nov 2025*
