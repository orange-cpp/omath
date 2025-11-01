# `omath::collision::Ray` & `LineTracer` — Ray–Triangle intersection (Möller–Trumbore)

> Headers: your project’s `ray.hpp` (includes `omath/linear_algebra/triangle.hpp`, `omath/linear_algebra/vector3.hpp`)
> Namespace: `omath::collision`
> Depends on: `omath::Vector3<float>`, `omath::Triangle<Vector3<float>>`
> Algorithm: **Möller–Trumbore** ray–triangle intersection (no allocation)

---

## Overview

These types provide a minimal, fast path to test and compute intersections between a **ray or line segment** and a **single triangle**:

* `Ray` — start/end points plus a flag to treat the ray as **infinite** (half-line) or a **finite segment**.
* `LineTracer` — static helpers:

    * `can_trace_line(ray, triangle)` → `true` if they intersect.
    * `get_ray_hit_point(ray, triangle)` → the hit point (precondition: intersection exists).

---

## `Ray`

```cpp
class Ray {
public:
  omath::Vector3<float> start;           // ray origin
  omath::Vector3<float> end;             // end point (for finite segment) or a point along the direction
  bool                  infinite_length = false;

  [[nodiscard]] omath::Vector3<float> direction_vector() const noexcept;
  [[nodiscard]] omath::Vector3<float> direction_vector_normalized() const noexcept;
};
```

### Semantics

* **Direction**: `direction_vector() == end - start`.
  The normalized variant returns a unit vector (or `{0,0,0}` if the direction length is zero).
* **Extent**:

    * `infinite_length == true` → treat as a **semi-infinite ray** from `start` along `direction`.
    * `infinite_length == false` → treat as a **closed segment** from `start` to `end`.

> Tip: For an infinite ray that points along some vector `d`, set `end = start + d`.

---

## `LineTracer`

```cpp
class LineTracer {
public:
  LineTracer() = delete;

  [[nodiscard]]
  static bool can_trace_line(
      const Ray& ray,
      const omath::Triangle<omath::Vector3<float>>& triangle
  ) noexcept;

  // Möller–Trumbore intersection
  [[nodiscard]]
  static omath::Vector3<float> get_ray_hit_point(
      const Ray& ray,
      const omath::Triangle<omath::Vector3<float>>& triangle
  ) noexcept;
};
```

### Behavior & contract

* **Intersection test**: `can_trace_line` returns `true` iff the ray/segment intersects the triangle (within the ray’s extent).
* **Hit point**: `get_ray_hit_point` **assumes** there is an intersection.
  Call **only after** `can_trace_line(...) == true`. Otherwise the result is unspecified.
* **Triangle winding**: Standard Möller–Trumbore works with either winding; no backface culling is implied here.
* **Degenerate inputs**: A zero-length ray or degenerate triangle yields **no hit** under typical Möller–Trumbore tolerances.

---

## Quick examples

### 1) Segment vs triangle

```cpp
using omath::Vector3;
using omath::Triangle;
using omath::collision::Ray;
using omath::collision::LineTracer;

Triangle<Vector3<float>> tri(
  Vector3<float>{0, 0, 0},
  Vector3<float>{1, 0, 0},
  Vector3<float>{0, 1, 0}
);

Ray seg;
seg.start = {0.25f, 0.25f, 1.0f};
seg.end   = {0.25f, 0.25f,-1.0f};
seg.infinite_length = false; // finite segment

if (LineTracer::can_trace_line(seg, tri)) {
  Vector3<float> hit = LineTracer::get_ray_hit_point(seg, tri);
  // use hit
}
```

### 2) Infinite ray

```cpp
Ray ray;
ray.start = {0.5f, 0.5f,  1.0f};
ray.end   = ray.start + Vector3<float>{0, 0, -1}; // direction only
ray.infinite_length = true;

bool hit = LineTracer::can_trace_line(ray, tri);
```

---

## Notes & edge cases

* **Normalization**: `direction_vector_normalized()` returns `{0,0,0}` for a zero-length direction (safe, but unusable for tracing).
* **Precision**: The underlying algorithm uses EPS thresholds to reject nearly parallel cases; results near edges can be sensitive to floating-point error. If you need robust edge inclusion/exclusion, document and enforce a policy (e.g., inclusive barycentric range with small epsilon).
* **Hit location**: The point returned by `get_ray_hit_point` lies **on the triangle plane** and within its area by construction (when `can_trace_line` is `true`).

---

## API summary

```cpp
namespace omath::collision {

class Ray {
public:
  Vector3<float> start, end;
  bool infinite_length = false;

  [[nodiscard]] Vector3<float> direction_vector() const noexcept;
  [[nodiscard]] Vector3<float> direction_vector_normalized() const noexcept;
};

class LineTracer {
public:
  LineTracer() = delete;

  [[nodiscard]] static bool can_trace_line(
    const Ray&,
    const omath::Triangle<omath::Vector3<float>>&
  ) noexcept;

  [[nodiscard]] static Vector3<float> get_ray_hit_point(
    const Ray&,
    const omath::Triangle<omath::Vector3<float>>&
  ) noexcept; // precondition: can_trace_line(...) == true
};

} // namespace omath::collision
```

---

## Implementation hints (if you extend it)

* Expose a variant that returns **barycentric coordinates** `(u, v, w)` alongside the hit point to support texture lookup or edge tests.
* Provide an overload returning `std::optional<Vector3<float>>` (or `expected`) for safer one-shot queries without a separate test call.
* If you need backface culling, add a flag or dedicated function (reject hits where the signed distance is negative with respect to triangle normal).

---

## See Also

- [Plane Documentation](../3d_primitives/plane.md) - Ray-plane intersection
- [Box Documentation](../3d_primitives/box.md) - AABB collision detection
- [Triangle Documentation](../linear_algebra/triangle.md) - Triangle primitives
- [Tutorials - Collision Detection](../tutorials.md#tutorial-4-collision-detection) - Complete collision tutorial
- [Getting Started Guide](../getting_started.md) - Quick start with OMath

---

*Last updated: 1 Nov 2025*
