# `omath::primitives::create_plane` — Build an oriented quad (2 triangles)

> Header: your project’s `primitives/plane.hpp`
> Namespace: `omath::primitives`
> Depends on: `omath::Triangle<omath::Vector3<float>>`, `omath::Vector3<float>`

```cpp
[[nodiscard]]
std::array<Triangle<Vector3<float>>, 2>
create_plane(const Vector3<float>& vertex_a,
             const Vector3<float>& vertex_b,
             const Vector3<float>& direction,
             float size) noexcept;
```

---

## What it does

Creates a **rectangle (quad)** in 3D oriented by the edge **A→B** and a second in-plane **direction**. The quad is returned as **two triangles** suitable for rendering or collision.

* Edge axis: `e = vertex_b - vertex_a`
* Width axis: “direction”, **projected to be perpendicular to `e`** so the quad is planar and well-formed.
* Normal (by right-hand rule): `n ∝ e × width`.

> **Sizing convention**
> Typical construction uses **half-width = `size`** along the (normalized, orthogonalized) *direction*, i.e. the total width is `2*size`.
> If your implementation interprets `size` as full width, adjust your expectations accordingly.

---

## Parameters

* `vertex_a`, `vertex_b` — two adjacent quad vertices defining the **long edge** of the plane.
* `direction` — a vector indicating the **cross-edge direction** within the plane (does not need to be orthogonal or normalized).
* `size` — **half-width** of the quad along the (processed) `direction`.

---

## Return

`std::array<Triangle<Vector3<float>>, 2>` — the quad triangulated (consistent CCW winding, outward normal per `e × width`).

---

## Robust construction (expected math)

1. `e = vertex_b - vertex_a`
2. Make `d` perpendicular to `e`:

   ```
   d = direction - e * (e.dot(direction) / e.length_sqr());
   if (d.length_sqr() == 0) pick an arbitrary perpendicular to e
   d = d.normalized();
   ```
3. Offsets: `w = d * size`
4. Four corners:

   ```
   A0 = vertex_a - w;  A1 = vertex_a + w;
   B0 = vertex_b - w;  B1 = vertex_b + w;
   ```
5. Triangles (CCW when viewed from +normal):

   ```
   T0 = Triangle{ A0, A1, B1 }
   T1 = Triangle{ A0, B1, B0 }
   ```

---

## Example

```cpp
using omath::Vector3;
using omath::Triangle;
using omath::primitives::create_plane;

Vector3<float> a{ -1, 0, -1 };    // edge start
Vector3<float> b{  1, 0, -1 };    // edge end
Vector3<float> dir{ 0, 0, 1 };    // cross-edge direction within the plane (roughly +Z)
float half_width = 2.0f;

auto quad = create_plane(a, b, dir, half_width);

// e.g., compute normals
for (const auto& tri : quad) {
  auto n = tri.calculate_normal(); (void)n;
}
```

---

## Notes & edge cases

* **Degenerate edge**: if `vertex_a == vertex_b`, the plane is undefined.
* **Collinearity**: if `direction` is parallel to `vertex_b - vertex_a`, the function must choose an alternate perpendicular; expect a fallback.
* **Winding**: If your renderer expects a specific face order, verify and swap the two vertices in each triangle as needed.
