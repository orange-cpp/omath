# `omath::primitives::create_box` — Build an oriented box as 12 triangles

> Header: your project’s `primitives/box.hpp` (declares `create_box`)
> Namespace: `omath::primitives`
> Depends on: `omath::Triangle<omath::Vector3<float>>`, `omath::Vector3<float>`

```cpp
[[nodiscard]]
std::array<Triangle<Vector3<float>>, 12>
create_box(const Vector3<float>& top,
           const Vector3<float>& bottom,
           const Vector3<float>& dir_forward,
           const Vector3<float>& dir_right,
           float ratio = 4.f) noexcept;
```

---

## What it does

Constructs a **rectangular cuboid (“box”)** oriented in 3D space and returns its surface as **12 triangles** (2 per face × 6 faces). The box’s central axis runs from `bottom` → `top`. The **up** direction is inferred from that segment; the **forward** and **right** directions define the box’s orientation around that axis.

The lateral half-extents are derived from the axis length and `ratio`:

> Let `H = |top - bottom|`. Lateral half-size ≈ `H / ratio` along both `dir_forward` and `dir_right`
> (i.e., the cross-section is a square of side `2H/ratio`).

> **Note:** This describes the intended behavior from the interface. If you rely on a different sizing rule, document it next to your implementation.

---

## Parameters

* `top`
  Center of the **top face**.

* `bottom`
  Center of the **bottom face**.

* `dir_forward`
  A direction that orients the box around its up axis. Should be **non-zero** and **not collinear** with `top - bottom`.

* `dir_right`
  A direction roughly orthogonal to both `dir_forward` and `top - bottom`. Used to fully fix orientation.

* `ratio` (default `4.0f`)
  Controls thickness relative to height. Larger values → thinner box.
  With the default rule above, half-extent = `|top-bottom|/ratio`.

---

## Return value

`std::array<Triangle<Vector3<float>>, 12>` — the six faces of the box, triangulated.
Winding is intended to be **outward-facing** (right-handed coordinates). Do not rely on a specific **face ordering**; treat the array as opaque unless your implementation guarantees an order.

---

## Expected math & robustness

* Define `u = normalize(top - bottom)`.
* Re-orthonormalize the basis to avoid skew:

  ```cpp
  f = normalize(dir_forward - u * u.dot(dir_forward));   // drop any up component
  r = normalize(u.cross(f));                              // right-handed basis
  // (Optionally recompute f = r.cross(u) for orthogonality)
  ```
* Half-extents: `h = length(top - bottom) / ratio;  hf = h * f;  hr = h * r`.
* Corners (top): `t±r±f = top ± hr ± hf`; (bottom): `b±r±f = bottom ± hr ± hf`.
* Triangulate each face with consistent CCW winding when viewed from outside.

---

## Example

```cpp
using omath::Vector3;
using omath::Triangle;
using omath::primitives::create_box;

// Axis from bottom to top (height 2)
Vector3<float> bottom{0, 0, 0};
Vector3<float> top    {0, 2, 0};

// Orientation around the axis
Vector3<float> forward{0, 0, 1};
Vector3<float> right  {1, 0, 0};

// Ratio 4 → lateral half-size = height/4 = 0.5
auto tris = create_box(top, bottom, forward, right, 4.0f);

// Use the triangles (normals, rendering, collision, etc.)
for (const auto& tri : tris) {
  auto n = tri.calculate_normal();
  (void)n;
}
```

---

## Usage notes & pitfalls

* **Degenerate axis**: If `top == bottom`, the box is undefined (zero height). Guard against this.
* **Directions**: Provide **non-zero**, **reasonably orthogonal** `dir_forward`/`dir_right`. A robust implementation should project/normalize internally, but callers should still pass sensible inputs.
* **Winding**: If your renderer or collision expects a specific winding, verify with a unit test and flip vertex order per face if necessary.
* **Thickness policy**: This doc assumes both lateral half-extents equal `|top-bottom|/ratio`. If your implementation diverges (e.g., separate forward/right ratios), document it.

---

## See also

* `omath::Triangle` (vertex utilities: normals, centroid, etc.)
* `omath::Vector3` (geometry operations used by the construction)

---

*Last updated: 31 Oct 2025*
