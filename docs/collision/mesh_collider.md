# `omath::collision::MeshCollider` — Convex hull collider for meshes

> Header: `omath/collision/mesh_collider.hpp`
> Namespace: `omath::collision`
> Depends on: `omath::primitives::Mesh`, `omath::Vector3<T>`
> Purpose: wrap a mesh to provide collision detection support for GJK/EPA

---

## Overview

`MeshCollider` wraps a `Mesh` object to provide the **support function** interface required by the GJK and EPA collision detection algorithms. The support function finds the vertex of the mesh farthest along a given direction, which is essential for constructing Minkowski difference simplices.

**Important**: `MeshCollider` assumes the mesh represents a **convex hull**. For non-convex shapes, you must either:
* Decompose into convex parts
* Use the convex hull of the mesh
* Use a different collision detection algorithm

---

## Template Declaration

```cpp
template<class MeshType>
class MeshCollider;
```

### MeshType Requirements

The `MeshType` must be an instantiation of `omath::primitives::Mesh` or provide:

```cpp
struct MeshType {
    using NumericType = /* float, double, etc. */;
    
    std::vector<Vector3<NumericType>> m_vertex_buffer;
    
    // Transform vertex from local to world space
    Vector3<NumericType> vertex_to_world_space(const Vector3<NumericType>&) const;
};
```

Common types:
* `omath::source_engine::Mesh`
* `omath::unity_engine::Mesh`
* `omath::unreal_engine::Mesh`
* `omath::frostbite_engine::Mesh`
* `omath::iw_engine::Mesh`
* `omath::opengl_engine::Mesh`

---

## Type Aliases

```cpp
using NumericType = typename MeshType::NumericType;
using VertexType = Vector3<NumericType>;
```

* `NumericType` — scalar type (typically `float`)
* `VertexType` — 3D vector type for vertices

---

## Constructor

```cpp
explicit MeshCollider(MeshType mesh);
```

Creates a collider from a mesh. The mesh is **moved** into the collider, so pass by value:

```cpp
omath::source_engine::Mesh my_mesh = /* ... */;
MeshCollider collider(std::move(my_mesh));
```

---

## Methods

### `find_furthest_vertex`

```cpp
[[nodiscard]]
const VertexType& find_furthest_vertex(const VertexType& direction) const;
```

Finds the vertex in the mesh's **local space** that has the maximum dot product with `direction`.

**Algorithm**: Linear search through all vertices (O(n) where n is vertex count).

**Returns**: Const reference to the vertex in `m_vertex_buffer`.

---

### `find_abs_furthest_vertex`

```cpp
[[nodiscard]]
VertexType find_abs_furthest_vertex(const VertexType& direction) const;
```

Finds the vertex farthest along `direction` and transforms it to **world space**. This is the primary method used by GJK/EPA.

**Steps**:
1. Find furthest vertex in local space using `find_furthest_vertex`
2. Transform to world space using `mesh.vertex_to_world_space()`

**Returns**: Vertex position in world coordinates.

**Usage in GJK**:
```cpp
// GJK support function for Minkowski difference
VertexType support = collider_a.find_abs_furthest_vertex(direction) 
                   - collider_b.find_abs_furthest_vertex(-direction);
```

---

## Usage Examples

### Basic Collision Detection

```cpp
using namespace omath::collision;
using namespace omath::source_engine;

// Create meshes with vertex data
std::vector<Vector3<float>> vbo_a = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1,  1}, {1, -1,  1}, {1, 1,  1}, {-1, 1,  1}
};
std::vector<Vector3<std::size_t>> vao_a = /* face indices */;

Mesh mesh_a(vbo_a, vao_a);
mesh_a.set_origin({0, 0, 0});

Mesh mesh_b(vbo_b, vao_b);
mesh_b.set_origin({5, 0, 0});  // Positioned away from mesh_a

// Wrap in colliders
MeshCollider<Mesh> collider_a(std::move(mesh_a));
MeshCollider<Mesh> collider_b(std::move(mesh_b));

// Run GJK
auto result = GjkAlgorithm<MeshCollider<Mesh>>::check_collision(
    collider_a, collider_b
);

if (result.hit) {
    std::cout << "Collision detected!\n";
}
```

### With EPA for Penetration Depth

```cpp
auto gjk_result = GjkAlgorithm<MeshCollider<Mesh>>::check_collision(
    collider_a, collider_b
);

if (gjk_result.hit) {
    auto epa_result = Epa<MeshCollider<Mesh>>::solve(
        collider_a, collider_b, gjk_result.simplex
    );
    
    if (epa_result.success) {
        std::cout << "Penetration: " << epa_result.depth << " units\n";
        std::cout << "Normal: " << epa_result.normal << "\n";
    }
}
```

### Custom Mesh Creation

```cpp
// Create a simple box mesh
std::vector<Vector3<float>> box_vertices = {
    {-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f},
    { 0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f},
    {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f},
    { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}
};

std::vector<Vector3<std::size_t>> box_indices = {
    {0, 1, 2}, {0, 2, 3},  // Front face
    {4, 6, 5}, {4, 7, 6},  // Back face
    {0, 4, 5}, {0, 5, 1},  // Bottom face
    {2, 6, 7}, {2, 7, 3},  // Top face
    {0, 3, 7}, {0, 7, 4},  // Left face
    {1, 5, 6}, {1, 6, 2}   // Right face
};

using namespace omath::source_engine;
Mesh box_mesh(box_vertices, box_indices);
box_mesh.set_origin({10, 0, 0});
box_mesh.set_scale({2, 2, 2});

MeshCollider<Mesh> box_collider(std::move(box_mesh));
```

### Oriented Collision

```cpp
// Create rotated mesh
Mesh mesh(vertices, indices);
mesh.set_origin({5, 5, 5});
mesh.set_scale({1, 1, 1});

// Set rotation (engine-specific angles)
ViewAngles rotation;
rotation.pitch = PitchAngle::from_degrees(45.0f);
rotation.yaw = YawAngle::from_degrees(30.0f);
mesh.set_rotation(rotation);

// Collider automatically handles transformation
MeshCollider<Mesh> collider(std::move(mesh));

// Support function returns world-space vertices
auto support = collider.find_abs_furthest_vertex({0, 1, 0});
```

---

## Performance Considerations

### Linear Search

`find_furthest_vertex` performs a **linear search** through all vertices:
* **Time complexity**: O(n) per support query
* **GJK iterations**: ~10-20 support queries per collision test
* **Total cost**: O(k × n) where k is GJK iterations

For meshes with many vertices (>1000), consider:
* Using simpler proxy geometry (bounding box, convex hull with fewer vertices)
* Pre-computing hierarchical structures
* Using specialized collision shapes when possible

### Caching Opportunities

The implementation uses `std::ranges::max_element`, which is cache-friendly for contiguous vertex buffers. For optimal performance:
* Store vertices contiguously in memory
* Avoid pointer-based or scattered vertex storage
* Consider SoA (Structure of Arrays) layout for SIMD optimization

### World Space Transformation

The `vertex_to_world_space` call involves matrix multiplication:
* **Cost**: ~15-20 floating-point operations per vertex
* **Optimization**: The mesh caches its transformation matrix
* **Update cost**: Only recomputed when origin/rotation/scale changes

---

## Limitations & Edge Cases

### Convex Hull Requirement

**Critical**: GJK/EPA only work with **convex shapes**. If your mesh is concave:

#### Option 1: Convex Decomposition
```cpp
// Decompose concave mesh into convex parts
std::vector<Mesh> convex_parts = decompose_mesh(concave_mesh);

for (const auto& part : convex_parts) {
    MeshCollider collider(part);
    // Test each part separately
}
```

#### Option 2: Use Convex Hull
```cpp
// Compute convex hull of vertices
auto hull_vertices = compute_convex_hull(mesh.m_vertex_buffer);
Mesh hull_mesh(hull_vertices, hull_indices);
MeshCollider collider(std::move(hull_mesh));
```

#### Option 3: Different Algorithm
Use triangle-based collision (e.g., LineTracer) for true concave support.

### Empty Mesh

Behavior is undefined if `m_vertex_buffer` is empty. Always ensure:
```cpp
assert(!mesh.m_vertex_buffer.empty());
MeshCollider collider(std::move(mesh));
```

### Degenerate Meshes

* **Single vertex**: Treated as a point (degenerates to sphere collision)
* **Two vertices**: Line segment (may cause GJK issues)
* **Coplanar vertices**: Flat mesh; EPA may have convergence issues

**Recommendation**: Use at least 4 non-coplanar vertices for robustness.

---

## Coordinate Systems

`MeshCollider` supports different engine coordinate systems through the `MeshTrait`:

| Engine | Up Axis | Handedness | Rotation Order |
|--------|---------|------------|----------------|
| Source Engine | Z | Right-handed | Pitch/Yaw/Roll |
| Unity | Y | Left-handed | Pitch/Yaw/Roll |
| Unreal | Z | Left-handed | Roll/Pitch/Yaw |
| Frostbite | Y | Right-handed | Pitch/Yaw/Roll |
| IW Engine | Z | Right-handed | Pitch/Yaw/Roll |
| OpenGL | Y | Right-handed | Pitch/Yaw/Roll |

The `vertex_to_world_space` method handles these differences transparently.

---

## Advanced Usage

### Custom Support Function

For specialized collision shapes, implement a custom collider:

```cpp
class SphereCollider {
public:
    using VertexType = Vector3<float>;
    
    Vector3<float> center;
    float radius;
    
    VertexType find_abs_furthest_vertex(const VertexType& direction) const {
        auto normalized = direction.normalized();
        return center + normalized * radius;
    }
};

// Use with GJK/EPA
auto result = GjkAlgorithm<SphereCollider>::check_collision(sphere_a, sphere_b);
```

### Debugging Support Queries

```cpp
class DebugMeshCollider : public MeshCollider<Mesh> {
public:
    using MeshCollider::MeshCollider;
    
    VertexType find_abs_furthest_vertex(const VertexType& direction) const {
        auto result = MeshCollider::find_abs_furthest_vertex(direction);
        std::cout << "Support query: direction=" << direction 
                  << " -> vertex=" << result << "\n";
        return result;
    }
};
```

---

## See Also

- [GJK Algorithm Documentation](gjk_algorithm.md) - Uses `MeshCollider` for collision detection
- [EPA Algorithm Documentation](epa_algorithm.md) - Uses `MeshCollider` for penetration depth
- [Simplex Documentation](simplex.md) - Data structure used by GJK
- [Mesh Documentation](../3d_primitives/mesh.md) - Underlying mesh primitive
- [Tutorials - Collision Detection](../tutorials.md#tutorial-4-collision-detection) - Complete collision tutorial

---

*Last updated: 24 Dec 2025*
