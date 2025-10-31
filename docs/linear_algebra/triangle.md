# `omath::Triangle` — Simple 3D triangle utility

> Header: your project’s `triangle.hpp`
> Namespace: `omath`
> Depends on: `omath::Vector3<float>` (from `vector3.hpp`)

A tiny helper around three `Vector3<float>` vertices with convenience methods for normals, edge vectors/lengths, a right-angle test (at **`v2`**), and the triangle centroid.

> **Note on the template parameter**
>
> The class is declared as `template<class Vector> class Triangle`, but the stored vertices are concretely `Vector3<float>`. In practice this type is currently **fixed to `Vector3<float>`**. You can ignore the template parameter or refactor to store `Vector` if you intend true genericity.

---

## Vertex layout & naming

```
v1
|\
| \
a |  \  hypot = |v1 - v3|
|   \
v2 -- v3
   b

a = |v1 - v2|  (side_a_length)
b = |v3 - v2|  (side_b_length)
```

* **`side_a_vector()`**  = `v1 - v2` (points from v2 → v1)
* **`side_b_vector()`**  = `v3 - v2` (points from v2 → v3)
* **Right-angle check** uses `a² + b² ≈ hypot²` with an epsilon of `1e-4`.

---

## Quick start

```cpp
#include "triangle.hpp"
using omath::Vector3;
using omath::Triangle;

Triangle<void> tri(                       // template arg unused; any placeholder ok
  Vector3<float>{0,0,0},                  // v1
  Vector3<float>{0,0,1},                  // v2  (right angle is tested at v2)
  Vector3<float>{1,0,1}                   // v3
);

auto n     = tri.calculate_normal();      // unit normal (right-handed: (v3-v2) × (v1-v2))
float a    = tri.side_a_length();         // |v1 - v2|
float b    = tri.side_b_length();         // |v3 - v2|
float hyp  = tri.hypot();                 // |v1 - v3|
bool rect  = tri.is_rectangular();        // true if ~right triangle at v2
auto C     = tri.mid_point();             // centroid (average of v1,v2,v3)
```

---

## Data members

```cpp
Vector3<float> m_vertex1;   // v1
Vector3<float> m_vertex2;   // v2 (the corner tested by is_rectangular)
Vector3<float> m_vertex3;   // v3
```

---

## Constructors

```cpp
constexpr Triangle() = default;
constexpr Triangle(const Vector3<float>& v1,
                   const Vector3<float>& v2,
                   const Vector3<float>& v3);
```

---

## Methods

```cpp
// Normal (unit) using right-handed cross product:
// n = (v3 - v2) × (v1 - v2), then normalized()
[[nodiscard]] constexpr Vector3<float> calculate_normal() const;

// Edge lengths with the naming from the diagram
[[nodiscard]] float side_a_length() const;  // |v1 - v2|
[[nodiscard]] float side_b_length() const;  // |v3 - v2|

// Edge vectors (from v2 to the other vertex)
[[nodiscard]] constexpr Vector3<float> side_a_vector() const; // v1 - v2
[[nodiscard]] constexpr Vector3<float> side_b_vector() const; // v3 - v2

// Hypotenuse length between v1 and v3
[[nodiscard]] constexpr float hypot() const;                  // |v1 - v3|

// Right-triangle check at vertex v2 (Pythagoras with epsilon 1e-4)
[[nodiscard]] constexpr bool is_rectangular() const;

// Centroid of the triangle (average of the 3 vertices)
[[nodiscard]] constexpr Vector3<float> mid_point() const;     // actually the centroid
```

### Notes & edge cases

* **Normal direction** follows the right-hand rule for the ordered vertices `{v2 → v3} × {v2 → v1}`.
  Swap vertex order to flip the normal.
* **Degenerate triangles** (collinear or overlapping vertices) yield a **zero vector** normal (since `normalized()` of the zero vector returns the zero vector in your math types).
* **`mid_point()` is the centroid**, not the midpoint of any single edge. If you need the midpoint of edge `v1–v2`, use `(m_vertex1 + m_vertex2) * 0.5f`.

---

## Examples

### Area and plane from existing API

```cpp
const auto a = tri.side_a_vector();
const auto b = tri.side_b_vector();
const auto n = b.cross(a);             // unnormalized normal
float area   = 0.5f * n.length();      // triangle area

// Plane equation n̂·(x - v2) = 0
auto nhat = n.length() > 0 ? n / n.length() : n;
float d = -nhat.dot(tri.m_vertex2);
```

### Project a point onto the triangle’s plane

```cpp
Vector3<float> p{0.3f, 1.0f, 0.7f};
auto n = tri.calculate_normal();
float t = n.dot(tri.m_vertex2 - p);     // signed distance along normal
auto projected = p + n * t;             // on-plane projection
```

---

## API summary (signatures)

```cpp
class Triangle final {
public:
  constexpr Triangle();
  constexpr Triangle(const Vector3<float>& v1,
                     const Vector3<float>& v2,
                     const Vector3<float>& v3);

  Vector3<float> m_vertex1, m_vertex2, m_vertex3;

  [[nodiscard]] constexpr Vector3<float> calculate_normal() const;
  [[nodiscard]] float side_a_length() const;
  [[nodiscard]] float side_b_length() const;
  [[nodiscard]] constexpr Vector3<float> side_a_vector() const;
  [[nodiscard]] constexpr Vector3<float> side_b_vector() const;
  [[nodiscard]] constexpr float hypot() const;
  [[nodiscard]] constexpr bool is_rectangular() const;
  [[nodiscard]] constexpr Vector3<float> mid_point() const;
};
```

---

## Suggestions (optional improvements)

* If generic vectors are intended, store `Vector m_vertex*;` and constrain `Vector` to the required ops (`-`, `cross`, `normalized`, `distance_to`, `+`, `/`).
* Consider renaming `mid_point()` → `centroid()` to avoid ambiguity.
* Expose an `area()` helper and (optionally) a barycentric coordinate routine if you plan to use this in rasterization or intersection tests.

---

*Last updated: 31 Oct 2025*
