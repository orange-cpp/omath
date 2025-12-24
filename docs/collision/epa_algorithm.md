# `omath::collision::Epa` — Expanding Polytope Algorithm for penetration depth

> Header: `omath/collision/epa_algorithm.hpp`
> Namespace: `omath::collision`
> Depends on: `Simplex<VertexType>`, collider types with `find_abs_furthest_vertex` method
> Algorithm: **EPA** (Expanding Polytope Algorithm) for penetration depth and contact normal

---

## Overview

The **EPA (Expanding Polytope Algorithm)** calculates the **penetration depth** and **separation normal** between two intersecting convex shapes. It is typically used as a follow-up to the GJK algorithm after a collision has been detected.

EPA takes a 4-point simplex containing the origin (from GJK) and iteratively expands it to find the point on the Minkowski difference closest to the origin. This point gives both:
* **Depth**: minimum translation distance to separate the shapes
* **Normal**: direction of separation (pointing from shape B to shape A)

`Epa` is a template class working with any collider type that implements the support function interface.

---

## `Epa::Result`

```cpp
struct Result final {
    bool success{false};        // true if EPA converged
    Vertex normal{};            // outward normal (from B to A)
    float depth{0.0f};          // penetration depth
    int iterations{0};          // number of iterations performed
    int num_vertices{0};        // final polytope vertex count
    int num_faces{0};           // final polytope face count
};
```

### Fields

* `success` — `true` if EPA successfully computed depth and normal; `false` if it failed to converge
* `normal` — unit vector pointing from shape B toward shape A (separation direction)
* `depth` — minimum distance to move shape A along `normal` to separate the shapes
* `iterations` — actual iteration count (useful for performance tuning)
* `num_vertices`, `num_faces` — final polytope size (for diagnostics)

---

## `Epa::Params`

```cpp
struct Params final {
    int max_iterations{64};     // maximum iterations before giving up
    float tolerance{1e-4f};     // absolute tolerance on distance growth
};
```

### Fields

* `max_iterations` — safety limit to prevent infinite loops (default 64)
* `tolerance` — convergence threshold: stop when distance grows less than this (default 1e-4)

---

## `Epa` Template Class

```cpp
template<class ColliderType>
class Epa final {
public:
    using Vertex = typename ColliderType::VertexType;
    static_assert(EpaVector<Vertex>, "VertexType must satisfy EpaVector concept");

    // Solve for penetration depth and normal
    [[nodiscard]]
    static Result solve(
        const ColliderType& a,
        const ColliderType& b,
        const Simplex<Vertex>& simplex,
        const Params params = {}
    );
};
```

### Precondition

The `simplex` parameter must:
* Have exactly 4 points (`simplex.size() == 4`)
* Contain the origin (i.e., be a valid GJK result with `hit == true`)

Violating this precondition leads to undefined behavior.

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

---

## Algorithm Details

### Expanding Polytope

EPA maintains a convex polytope (polyhedron) in Minkowski difference space `A - B`. Starting from the 4-point tetrahedron (simplex from GJK), it repeatedly:

1. **Find closest face** to the origin
2. **Support query** in the direction of the face normal
3. **Expand polytope** by adding the new support point
4. **Update faces** to maintain convexity

The algorithm terminates when:
* **Convergence**: the distance from origin to polytope stops growing (within tolerance)
* **Max iterations**: safety limit reached
* **Failure cases**: degenerate polytope or numerical issues

### Minkowski Difference

Like GJK, EPA operates in Minkowski difference space where `point = a - b` for points in shapes A and B. The closest point on this polytope to the origin gives the minimum separation.

### Face Winding

Faces are stored with outward-pointing normals. The algorithm uses a priority queue to efficiently find the face closest to the origin.

---

## Vertex Type Requirements

The `VertexType` must satisfy the `EpaVector` concept:

```cpp
template<class V>
concept EpaVector = requires(const V& a, const V& b, float s) {
    { a - b } -> std::same_as<V>;
    { a.cross(b) } -> std::same_as<V>;
    { a.dot(b) } -> std::same_as<float>;
    { -a } -> std::same_as<V>;
    { a * s } -> std::same_as<V>;
    { a / s } -> std::same_as<V>;
};
```

`omath::Vector3<float>` satisfies this concept.

---

## Usage Examples

### Basic EPA Usage

```cpp
using namespace omath::collision;
using namespace omath::source_engine;

// First, run GJK to detect collision
MeshCollider<Mesh> collider_a(mesh_a);
MeshCollider<Mesh> collider_b(mesh_b);

auto gjk_result = GjkAlgorithm<MeshCollider<Mesh>>::check_collision(
    collider_a,
    collider_b
);

if (gjk_result.hit) {
    // Collision detected, use EPA to get penetration info
    auto epa_result = Epa<MeshCollider<Mesh>>::solve(
        collider_a,
        collider_b,
        gjk_result.simplex
    );
    
    if (epa_result.success) {
        std::cout << "Penetration depth: " << epa_result.depth << "\n";
        std::cout << "Separation normal: " 
                  << "(" << epa_result.normal.x << ", "
                  << epa_result.normal.y << ", "
                  << epa_result.normal.z << ")\n";
                  
        // Apply separation: move A away from B
        Vector3<float> correction = epa_result.normal * epa_result.depth;
        mesh_a.set_origin(mesh_a.get_origin() + correction);
    }
}
```

### Custom Parameters

```cpp
// Use custom convergence settings
Epa<Collider>::Params params;
params.max_iterations = 128;    // Allow more iterations for complex shapes
params.tolerance = 1e-5f;       // Tighter tolerance for more accuracy

auto result = Epa<Collider>::solve(a, b, simplex, params);
```

### Physics Integration

```cpp
void resolve_collision(PhysicsBody& body_a, PhysicsBody& body_b) {
    auto gjk_result = GjkAlgorithm<Collider>::check_collision(
        body_a.collider, body_b.collider
    );
    
    if (!gjk_result.hit)
        return;  // No collision
    
    auto epa_result = Epa<Collider>::solve(
        body_a.collider,
        body_b.collider,
        gjk_result.simplex
    );
    
    if (epa_result.success) {
        // Separate bodies
        float mass_sum = body_a.mass + body_b.mass;
        float ratio_a = body_b.mass / mass_sum;
        float ratio_b = body_a.mass / mass_sum;
        
        body_a.position += epa_result.normal * (epa_result.depth * ratio_a);
        body_b.position -= epa_result.normal * (epa_result.depth * ratio_b);
        
        // Apply collision response
        apply_impulse(body_a, body_b, epa_result.normal);
    }
}
```

---

## Performance Characteristics

* **Time complexity**: O(k × f) where k is iterations and f is faces per iteration (typically f grows slowly)
* **Space complexity**: O(n) where n is the number of polytope vertices (typically < 100)
* **Typical iterations**: 4-20 for most collisions
* **Worst case**: 64 iterations (configurable limit)

### Performance Tips

1. **Adjust max_iterations**: Balance accuracy vs. performance for your use case
2. **Tolerance tuning**: Larger tolerance = faster convergence but less accurate
3. **Shape complexity**: Simpler shapes (fewer faces) converge faster
4. **Deep penetrations**: Require more iterations; consider broad-phase separation

---

## Limitations & Edge Cases

* **Requires valid simplex**: Must be called with a 4-point simplex containing the origin (from successful GJK)
* **Convex shapes only**: Like GJK, EPA only works with convex colliders
* **Convergence failure**: Can fail to converge for degenerate or very thin shapes (check `result.success`)
* **Numerical precision**: Extreme scale differences or very small shapes may cause issues
* **Deep penetration**: Very deep intersections may require many iterations or fail to converge

### Error Handling

```cpp
auto result = Epa<Collider>::solve(a, b, simplex);

if (!result.success) {
    // EPA failed to converge
    // Fallback options:
    // 1. Use a default separation (e.g., axis between centers)
    // 2. Increase max_iterations and retry
    // 3. Log a warning and skip this collision
    std::cerr << "EPA failed after " << result.iterations << " iterations\n";
}
```

---

## Theory & Background

### Why EPA after GJK?

GJK determines **if** shapes intersect but doesn't compute penetration depth. EPA extends GJK's final simplex to find the exact depth and normal needed for:
* **Collision response** — separating objects realistically
* **Contact manifolds** — generating contact points for physics
* **Constraint solving** — iterative physics solvers

### Comparison with SAT

| Feature | EPA | SAT (Separating Axis Theorem) |
|---------|-----|-------------------------------|
| Works with | Any convex shape | Polytopes (faces/edges) |
| Penetration depth | Yes | Yes |
| Complexity | Iterative | Per-axis projection |
| Best for | General convex | Boxes, prisms |
| Typical speed | Moderate | Fast (few axes) |

EPA is more general; SAT is faster for axis-aligned shapes.

---

## Implementation Details

The EPA implementation in OMath:
* Uses a **priority queue** to efficiently find the closest face
* Maintains face winding for consistent normals
* Handles **edge cases**: degenerate faces, numerical instability
* Prevents infinite loops with iteration limits
* Returns detailed diagnostics (iteration count, polytope size)

---

## See Also

- [GJK Algorithm Documentation](gjk_algorithm.md) - Collision detection (required before EPA)
- [Simplex Documentation](simplex.md) - Input simplex structure
- [MeshCollider Documentation](mesh_collider.md) - Mesh-based collider
- [Mesh Documentation](../3d_primitives/mesh.md) - Mesh primitive
- [Tutorials - Collision Detection](../tutorials.md#tutorial-4-collision-detection) - Complete collision tutorial
- [API Overview](../api_overview.md) - High-level API reference

---

*Last updated: 24 Dec 2025*
