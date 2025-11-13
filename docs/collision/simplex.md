# `omath::collision::Simplex` — Fixed-capacity simplex for GJK/EPA

> Header: `omath/collision/simplex.hpp`
> Namespace: `omath::collision`
> Depends on: `Vector3<float>` (or any type satisfying `GjkVector` concept)
> Purpose: store and manipulate simplices in GJK and EPA algorithms

---

## Overview

`Simplex` is a lightweight container for up to 4 points, used internally by the GJK and EPA collision detection algorithms. A simplex in this context is a geometric shape defined by 1 to 4 vertices:

* **1 point** — a single vertex
* **2 points** — a line segment
* **3 points** — a triangle
* **4 points** — a tetrahedron

The GJK algorithm builds simplices incrementally to detect collisions, and EPA extends a 4-point simplex to compute penetration depth.

---

## Template & Concepts

```cpp
template<GjkVector VectorType = Vector3<float>>
class Simplex final;
```

### `GjkVector` Concept

The vertex type must satisfy:

```cpp
template<class V>
concept GjkVector = requires(const V& a, const V& b) {
    { -a } -> std::same_as<V>;
    { a - b } -> std::same_as<V>;
    { a.cross(b) } -> std::same_as<V>;
    { a.point_to_same_direction(b) } -> std::same_as<bool>;
};
```

`omath::Vector3<float>` satisfies this concept and is the default.

---

## Constructors & Assignment

```cpp
constexpr Simplex() = default;

constexpr Simplex& operator=(std::initializer_list<VectorType> list) noexcept;
```

### Initialization

```cpp
// Empty simplex
Simplex<Vector3<float>> s;

// Initialize with points
Simplex<Vector3<float>> s2;
s2 = {v1, v2, v3};  // 3-point simplex (triangle)
```

**Constraint**: Maximum 4 points. Passing more triggers an assertion in debug builds.

---

## Core Methods

### Adding Points

```cpp
constexpr void push_front(const VectorType& p) noexcept;
```

Inserts a point at the **front** (index 0), shifting existing points back. If the simplex is already at capacity (4 points), the last point is discarded.

**Usage pattern in GJK**:
```cpp
simplex.push_front(new_support_point);
// Now simplex[0] is the newest point
```

### Size & Capacity

```cpp
[[nodiscard]] constexpr std::size_t size() const noexcept;
[[nodiscard]] static constexpr std::size_t capacity = 4;
```

* `size()` — current number of points (0-4)
* `capacity` — maximum points (always 4)

### Element Access

```cpp
[[nodiscard]] constexpr VectorType& operator[](std::size_t index) noexcept;
[[nodiscard]] constexpr const VectorType& operator[](std::size_t index) const noexcept;
```

Access points by index. **No bounds checking** — index must be `< size()`.

```cpp
if (simplex.size() >= 2) {
    auto edge = simplex[1] - simplex[0];
}
```

### Iterators

```cpp
[[nodiscard]] constexpr auto begin() noexcept;
[[nodiscard]] constexpr auto end() noexcept;
[[nodiscard]] constexpr auto begin() const noexcept;
[[nodiscard]] constexpr auto end() const noexcept;
```

Standard iterator support for range-based loops:

```cpp
for (const auto& vertex : simplex) {
    std::cout << vertex << "\n";
}
```

---

## GJK-Specific Methods

These methods implement the core logic for simplifying simplices in the GJK algorithm.

### `contains_origin`

```cpp
[[nodiscard]] constexpr bool contains_origin() noexcept;
```

Determines if the origin lies within the current simplex. This is the **core GJK test**: if true, the shapes intersect.

* For a **1-point** simplex, always returns `false` (can't contain origin)
* For a **2-point** simplex (line), checks if origin projects onto the segment
* For a **3-point** simplex (triangle), checks if origin projects onto the triangle
* For a **4-point** simplex (tetrahedron), checks if origin is inside

**Side effect**: Simplifies the simplex by removing points not needed to maintain proximity to the origin. After calling, `size()` may have decreased.

**Return value**: 
* `true` — origin is contained (collision detected)
* `false` — origin not contained; simplex has been simplified toward origin

### `next_direction`

```cpp
[[nodiscard]] constexpr VectorType next_direction() const noexcept;
```

Computes the next search direction for GJK. This is the direction from the simplex toward the origin, used to query the next support point.

* Must be called **after** `contains_origin()` returns `false`
* Behavior is **undefined** if called when `size() == 0` or when origin is already contained

---

## Usage Examples

### GJK Iteration (Simplified)

```cpp
Simplex<Vector3<float>> simplex;
Vector3<float> direction{1, 0, 0};  // Initial search direction

for (int i = 0; i < 64; ++i) {
    // Get support point in current direction
    auto support = find_support_vertex(collider_a, collider_b, direction);
    
    // Check if we made progress
    if (support.dot(direction) <= 0)
        break;  // No collision possible
    
    simplex.push_front(support);
    
    // Check if simplex contains origin
    if (simplex.contains_origin()) {
        // Collision detected!
        assert(simplex.size() == 4);
        return GjkHitInfo{true, simplex};
    }
    
    // Get next search direction
    direction = simplex.next_direction();
}

// No collision
return GjkHitInfo{false, {}};
```

### Manual Simplex Construction

```cpp
using Vec3 = Vector3<float>;

Simplex<Vec3> simplex;
simplex = {
    Vec3{0.0f, 0.0f, 0.0f},
    Vec3{1.0f, 0.0f, 0.0f},
    Vec3{0.0f, 1.0f, 0.0f},
    Vec3{0.0f, 0.0f, 1.0f}
};

assert(simplex.size() == 4);

// Check if origin is inside this tetrahedron
bool has_collision = simplex.contains_origin();
```

### Iterating Over Points

```cpp
void print_simplex(const Simplex<Vector3<float>>& s) {
    std::cout << "Simplex with " << s.size() << " points:\n";
    for (std::size_t i = 0; i < s.size(); ++i) {
        const auto& p = s[i];
        std::cout << "  [" << i << "] = (" 
                  << p.x << ", " << p.y << ", " << p.z << ")\n";
    }
}
```

---

## Implementation Details

### Simplex Simplification

The `contains_origin()` method implements different tests based on simplex size:

#### Line Segment (2 points)

Checks if origin projects onto segment `[A, B]`:
* If yes, keeps both points
* If no, keeps only the closer point

#### Triangle (3 points)

Tests the origin against the triangle plane and edges using cross products. Simplifies to:
* The full triangle if origin projects onto its surface
* An edge if origin is closest to that edge
* A single vertex otherwise

#### Tetrahedron (4 points)

Tests origin against all four faces:
* If origin is inside, returns `true` (collision)
* If outside, reduces to the face/edge/vertex closest to origin

### Direction Calculation

The `next_direction()` method computes:
* For **line**: perpendicular from line toward origin
* For **triangle**: perpendicular from triangle toward origin
* Implementation uses cross products and projections to avoid sqrt when possible

---

## Performance Characteristics

* **Storage**: Fixed 4 × `sizeof(VectorType)` + size counter
* **Push front**: O(n) where n is current size (max 4, so effectively O(1))
* **Contains origin**: O(1) for each case (line, triangle, tetrahedron)
* **Next direction**: O(1) — simple cross products and subtractions
* **No heap allocations**: All storage is inline

**constexpr**: All methods are `constexpr`, enabling compile-time usage where feasible.

---

## Edge Cases & Constraints

### Degenerate Simplices

* **Zero-length edges**: Can occur if support points coincide. The algorithm handles this by checking `point_to_same_direction` before divisions.
* **Collinear points**: Triangle simplification detects and handles collinear cases by reducing to a line.
* **Flat tetrahedron**: If the 4th point is coplanar with the first 3, the origin containment test may have reduced precision.

### Assertions

* **Capacity**: `operator=` asserts `list.size() <= 4` in debug builds
* **Index bounds**: No bounds checking in release builds — ensure `index < size()`

### Thread Safety

* **Read-only**: Safe to read from multiple threads
* **Modification**: Not thread-safe; synchronize writes externally

---

## Relationship to GJK & EPA

### In GJK

* Starts empty or with an initial point
* Grows via `push_front` as support points are added
* Shrinks via `contains_origin` as it's simplified
* Once it reaches 4 points and contains origin, GJK succeeds

### In EPA

* Takes a 4-point simplex from GJK as input
* Uses the tetrahedron as the initial polytope
* Does not directly use the `Simplex` class for expansion (EPA maintains a more complex polytope structure)

---

## See Also

- [GJK Algorithm Documentation](gjk_algorithm.md) - Uses `Simplex` for collision detection
- [EPA Algorithm Documentation](epa_algorithm.md) - Takes 4-point `Simplex` as input
- [MeshCollider Documentation](mesh_collider.md) - Provides support function for GJK/EPA
- [Vector3 Documentation](../linear_algebra/vector3.md) - Default vertex type
- [Tutorials - Collision Detection](../tutorials.md#tutorial-4-collision-detection) - Collision tutorial

---

*Last updated: 13 Nov 2025*
