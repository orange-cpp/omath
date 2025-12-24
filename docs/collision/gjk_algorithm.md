# `omath::collision::GjkAlgorithm` — Gilbert-Johnson-Keerthi collision detection

> Header: `omath/collision/gjk_algorithm.hpp`
> Namespace: `omath::collision`
> Depends on: `Simplex<VertexType>`, collider types with `find_abs_furthest_vertex` method
> Algorithm: **GJK** (Gilbert-Johnson-Keerthi) for convex shape collision detection

---

## Overview

The **GJK algorithm** determines whether two convex shapes intersect by iteratively constructing a simplex in Minkowski difference space. The algorithm is widely used in physics engines and collision detection systems due to its efficiency and robustness.

`GjkAlgorithm` is a template class that works with any collider type implementing the required support function interface:

* `find_abs_furthest_vertex(direction)` — returns the farthest point in the collider along the given direction.

The algorithm returns a `GjkHitInfo` containing:
* `hit` — boolean indicating whether the shapes intersect
* `simplex` — a 4-point simplex containing the origin (valid only when `hit == true`)

---

## `GjkHitInfo`

```cpp
template<class VertexType>
struct GjkHitInfo final {
    bool hit{false};                    // true if collision detected
    Simplex<VertexType> simplex;        // 4-point simplex (valid only if hit == true)
};
```

The `simplex` field is only meaningful when `hit == true` and contains 4 points. This simplex can be passed to the EPA algorithm for penetration depth calculation.

---

## `GjkAlgorithm`

```cpp
template<class ColliderType>
class GjkAlgorithm final {
    using VertexType = typename ColliderType::VertexType;

public:
    // Find support vertex in Minkowski difference
    [[nodiscard]]
    static VertexType find_support_vertex(
        const ColliderType& collider_a,
        const ColliderType& collider_b,
        const VertexType& direction
    );

    // Check if two convex shapes intersect
    [[nodiscard]]
    static GjkHitInfo<VertexType> check_collision(
        const ColliderType& collider_a,
        const ColliderType& collider_b
    );
};
```

---

## Collider Requirements

Any type used as `ColliderType` must provide:

```cpp
// Type alias for vertex type (typically Vector3<float>)
using VertexType = /* ... */;

// Find the farthest point in world space along the given direction
[[nodiscard]]
VertexType find_abs_furthest_vertex(const VertexType& direction) const;
```

Common collider types:
* `MeshCollider<MeshType>` — for arbitrary triangle meshes
* Custom colliders for spheres, boxes, capsules, etc.

---

## Algorithm Details

### Minkowski Difference

GJK operates in the **Minkowski difference** space `A - B`, where a point in this space represents the difference between points in shapes A and B. The shapes intersect if and only if the origin lies within this Minkowski difference.

### Support Function

The support function finds the point in the Minkowski difference farthest along a given direction:

```cpp
support(A, B, dir) = A.furthest(dir) - B.furthest(-dir)
```

This is computed by `find_support_vertex`.

### Simplex Iteration

The algorithm builds a simplex incrementally:
1. Start with an initial direction (typically vector between shape centers)
2. Add support vertices in directions that move the simplex toward the origin
3. Simplify the simplex to keep only points closest to the origin
4. Repeat until either:
   * Origin is contained (collision detected, returns 4-point simplex)
   * No progress can be made (no collision)

Maximum 64 iterations are performed to prevent infinite loops in edge cases.

---

## Usage Examples

### Basic Collision Check

```cpp
using namespace omath::collision;
using namespace omath::source_engine;

// Create mesh colliders
Mesh mesh_a = /* ... */;
Mesh mesh_b = /* ... */;

MeshCollider collider_a(mesh_a);
MeshCollider collider_b(mesh_b);

// Check for collision
auto result = GjkAlgorithm<MeshCollider<Mesh>>::check_collision(
    collider_a,
    collider_b
);

if (result.hit) {
    std::cout << "Collision detected!\n";
    // Can pass result.simplex to EPA for penetration depth
}
```

### Combined with EPA

```cpp
auto gjk_result = GjkAlgorithm<Collider>::check_collision(a, b);

if (gjk_result.hit) {
    // Get penetration depth and normal using EPA
    auto epa_result = Epa<Collider>::solve(
        a, b, gjk_result.simplex
    );
    
    if (epa_result.success) {
        std::cout << "Penetration depth: " << epa_result.depth << "\n";
        std::cout << "Separation normal: " << epa_result.normal << "\n";
    }
}
```

---

## Performance Characteristics

* **Time complexity**: O(k) where k is the number of iterations (typically < 20 for most cases)
* **Space complexity**: O(1) — only stores a 4-point simplex
* **Best case**: 4-8 iterations for well-separated objects
* **Worst case**: 64 iterations (hard limit)
* **Cache efficient**: operates on small fixed-size data structures

### Optimization Tips

1. **Initial direction**: Use vector between shape centers for faster convergence
2. **Early exit**: GJK quickly rejects non-intersecting shapes
3. **Warm starting**: Reuse previous simplex for continuous collision detection
4. **Broad phase**: Use spatial partitioning before GJK (AABB trees, grids)

---

## Limitations & Edge Cases

* **Convex shapes only**: GJK only works with convex colliders. For concave shapes, decompose into convex parts or use a mesh collider wrapper.
* **Degenerate simplices**: The algorithm handles degenerate cases, but numerical precision can cause issues with very thin or flat shapes.
* **Iteration limit**: Hard limit of 64 iterations prevents infinite loops but may miss collisions in extreme cases.
* **Zero-length directions**: The simplex update logic guards against zero-length vectors, returning safe fallbacks.

---

## Vertex Type Requirements

The `VertexType` must satisfy the `GjkVector` concept (defined in `simplex.hpp`):

```cpp
template<class V>
concept GjkVector = requires(const V& a, const V& b) {
    { -a } -> std::same_as<V>;
    { a - b } -> std::same_as<V>;
    { a.cross(b) } -> std::same_as<V>;
    { a.point_to_same_direction(b) } -> std::same_as<bool>;
};
```

`omath::Vector3<float>` satisfies this concept.

---

## See Also

- [EPA Algorithm Documentation](epa_algorithm.md) - Penetration depth calculation
- [Simplex Documentation](simplex.md) - Simplex data structure
- [MeshCollider Documentation](mesh_collider.md) - Mesh-based collider
- [Mesh Documentation](../3d_primitives/mesh.md) - Mesh primitive
- [LineTracer Documentation](line_tracer.md) - Ray-triangle intersection
- [Tutorials - Collision Detection](../tutorials.md#tutorial-4-collision-detection) - Complete collision tutorial

---

*Last updated: 24 Dec 2025*
