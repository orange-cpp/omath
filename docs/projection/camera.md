# `omath::projection::Camera` — Generic, trait-driven camera with screen/world conversion

> Header: `omath/projection/camera.hpp` (this header)
> Namespace: `omath::projection`
> Template: `Camera<Mat4X4Type, ViewAnglesType, TraitClass>`
> Requires: `CameraEngineConcept<TraitClass, Mat4X4Type, ViewAnglesType>`
> Key features: **lazy view-projection caching**, world↔screen helpers, pluggable math via a **Trait**

---

## Overview

`Camera` is a small, zero-allocation camera wrapper. It delegates the math for **view**, **projection**, and **look-at** to a **Trait** (`TraitClass`), which lets you plug in different coordinate systems or conventions without changing the camera code. The class caches the **View×Projection** matrix and invalidates it when any parameter changes.

Alongside the camera, the header defines:

* `struct ViewPort { float m_width, m_height; float aspect_ratio() const; }`
* `using FieldOfView = Angle<float, 0.f, 180.f, AngleFlags::Clamped>;`

---

## Template & trait requirements

```cpp
template<class T, class MatType, class ViewAnglesType>
concept CameraEngineConcept = requires(
  const omath::Vector3<float>& cam_origin,
  const omath::Vector3<float>& look_at,
  const ViewAnglesType& angles,
  const omath::projection::FieldOfView& fov,
  const omath::projection::ViewPort& viewport,
  float znear, float zfar
) {
  { T::calc_look_at_angle(cam_origin, look_at) }        noexcept -> std::same_as<ViewAnglesType>;
  { T::calc_view_matrix(angles, cam_origin) }            noexcept -> std::same_as<MatType>;
  { T::calc_projection_matrix(fov, viewport, znear, zfar)}noexcept -> std::same_as<MatType>;
};
```

Your `Mat4X4Type` must behave like the library’s `Mat<4,4,...>` (supports `*`, `/`, `inverted()`, `.at(r,c)`, `.raw_array()`, and `static constexpr get_store_ordering()`).

---

## Quick start

```cpp
using Mat4 = omath::Mat<4,4,float, omath::MatStoreType::COLUMN_MAJOR>;

// Example trait (sketch): assumes Y-up, column-major, left-handed
struct MyCamTrait {
  static ViewAnglesType calc_look_at_angle(const Vector3<float>& eye,
                                           const Vector3<float>& at) noexcept;
  static Mat4 calc_view_matrix(const ViewAnglesType& ang,
                               const Vector3<float>& eye) noexcept;
  static Mat4 calc_projection_matrix(const FieldOfView& fov,
                                     const ViewPort& vp,
                                     float znear, float zfar) noexcept;
};

using Camera = omath::projection::Camera<Mat4, MyViewAngles, MyCamTrait>;

omath::projection::ViewPort vp{1920, 1080};
omath::projection::FieldOfView fov = omath::angles::degrees(70.f);

Camera cam(/*position*/ {0,1.7f, -3},
           /*angles*/   MyViewAngles{/*...*/},
           /*viewport*/ vp, fov,
           /*near*/     0.1f,
           /*far*/      1000.f);

// Project world → screen (origin top-left)
auto s = cam.world_to_screen<Camera::ScreenStart::TOP_LEFT_CORNER>({1, 1, 0});
if (s) {
  // s->x, s->y in pixels; s->z in NDC depth
}
```

---

## API

```cpp
enum class ScreenStart { TOP_LEFT_CORNER, BOTTOM_LEFT_CORNER };

class Camera final {
public:
  ~Camera() = default;

  Camera(const Vector3<float>& position,
         const ViewAnglesType& view_angles,
         const ViewPort& view_port,
         const FieldOfView& fov,
         float near, float far) noexcept;

  void look_at(const Vector3<float>& target); // recomputes view angles; invalidates cache

  // Lazily computed and cached:
  const Mat4X4Type& get_view_projection_matrix() const noexcept;

  // Setters (all invalidate cached VP):
  void set_field_of_view(const FieldOfView&) noexcept;
  void set_near_plane(float) noexcept;
  void set_far_plane(float) noexcept;
  void set_view_angles(const ViewAnglesType&) noexcept;
  void set_origin(const Vector3<float>&) noexcept;
  void set_view_port(const ViewPort&) noexcept;

  // Getters:
  const FieldOfView&     get_field_of_view() const noexcept;
  const float&           get_near_plane()    const noexcept;
  const float&           get_far_plane()     const noexcept;
  const ViewAnglesType&  get_view_angles()   const noexcept;
  const Vector3<float>&  get_origin()        const noexcept;

  // World → Screen (pixels) via NDC; choose screen origin:
  template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
  std::expected<Vector3<float>, Error>
  world_to_screen(const Vector3<float>& world) const noexcept;

  // World → NDC (aka “viewport” in this code) ∈ [-1,1]^3
  std::expected<Vector3<float>, Error>
  world_to_view_port(const Vector3<float>& world) const noexcept;

  // NDC → World (uses inverse VP)
  std::expected<Vector3<float>, Error>
  view_port_to_screen(const Vector3<float>& ndc) const noexcept;

  // Screen (pixels) → World
  std::expected<Vector3<float>, Error>
  screen_to_world(const Vector3<float>& screen) const noexcept;

  // 2D overload (z defaults to 1, i.e., far plane ray-end in NDC)
  std::expected<Vector3<float>, Error>
  screen_to_world(const Vector2<float>& screen) const noexcept;

protected:
  ViewPort     m_view_port{};
  FieldOfView  m_field_of_view;
  mutable std::optional<Mat4X4Type> m_view_projection_matrix;
  float        m_far_plane_distance{};
  float        m_near_plane_distance{};
  ViewAnglesType m_view_angles;
  Vector3<float> m_origin;

private:
  static constexpr bool is_ndc_out_of_bounds(const Mat4X4Type& ndc) noexcept;
  Vector3<float> ndc_to_screen_position_from_top_left_corner(const Vector3<float>& ndc) const noexcept;
  Vector3<float> ndc_to_screen_position_from_bottom_left_corner(const Vector3<float>& ndc) const noexcept;
  Vector3<float> screen_to_ndc(const Vector3<float>& screen) const noexcept;
};
```

### Error handling

All conversions return `std::expected<..., Error>` with errors from `error_codes.hpp`, notably:

* `Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS` — clip space W=0 or NDC outside `[-1,1]`.
* `Error::INV_VIEW_PROJ_MAT_DET_EQ_ZERO` — non-invertible View×Projection matrix.

---

## Coordinate spaces & conversions

### World → NDC (`world_to_view_port`)

1. Build (or reuse cached) `VP = P * V` (projection * view).
2. Multiply by homogeneous column from the world point.
3. Reject if `w == 0`.
4. Perspective divide → NDC in `[-1,1]^3`.
5. Reject if any component is out of range.

Returns `{x_ndc, y_ndc, z_ndc}`.

### NDC → Screen (pixels)

The class offers two origins:

* **Top-left (default)**

  ```
  x_px = (x_ndc + 1)/2 * width
  y_px = ( -y_ndc/2 + 0.5) * height   // flips Y
  ```
* **Bottom-left**

  ```
  x_px = (x_ndc + 1)/2 * width
  y_px = ( y_ndc/2 + 0.5) * height
  ```

### Screen (pixels) → NDC

```
x_ndc =  screen_x / width  * 2 - 1
y_ndc =  1 - screen_y / height * 2   // Top-left screen origin assumed here
z_ndc =  screen_z                     // Caller-provided (e.g., 0..1 depth)
```

### NDC → World (`view_port_to_screen`)

Despite the method name, this function **unprojects** an NDC point back to world space:

1. Compute `VP^{-1}`; if not invertible → error.
2. Multiply by NDC (homogeneous 4D) and divide by `w`.
3. Return world point.

> Tip: to build a **world-space ray** from a screen pixel, unproject at `z=0` (near) and `z=1` (far).

---

## Caching & invalidation

* `get_view_projection_matrix()` computes `P*V` once and caches it.
* Any setter (`set_*`) or `look_at()` clears the cache (`m_view_projection_matrix = std::nullopt`).

---

## Notes & gotchas

* **Matrix order**: The camera multiplies `P * V`. Make sure your **Trait** matches this convention.
* **Store ordering**: The `Mat4X4Type::get_store_ordering()` is used when building homogeneous columns; ensure it’s consistent with your matrix implementation.
* **Naming quirk**: `view_port_to_screen()` returns a **world** point from **NDC** (it’s an unproject). Consider renaming to `ndc_to_world()` in your codebase for clarity.
* **FOV units**: `FieldOfView` uses the project’s `Angle` type; pass degrees via `angles::degrees(...)`.

---

## Minimal trait sketch (column-major, left-handed)

```cpp
struct LHCTrait {
  static MyAngles calc_look_at_angle(const Vector3<float>& eye,
                                     const Vector3<float>& at) noexcept { /* ... */ }

  static Mat4 calc_view_matrix(const MyAngles& ang,
                               const Vector3<float>& eye) noexcept {
    // Build from forward/right/up and translation
  }

  static Mat4 calc_projection_matrix(const FieldOfView& fov,
                                     const ViewPort& vp,
                                     float zn, float zf) noexcept {
    return omath::mat_perspective_left_handed<float, omath::MatStoreType::COLUMN_MAJOR>(
      fov.as_degrees(), vp.aspect_ratio(), zn, zf
    );
  }
};
```

---

## Testing checklist

* World point centered in view projects to **screen center**.
* Points outside frustum → `WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS`.
* Inverting `VP` fails gracefully for singular matrices.
* `ScreenStart` switch flips Y as expected.
* Screen→World ray: unproject `(x,y,0)` and `(x,y,1)` and verify direction passes through the camera frustum.

---

## See Also

- [Engine-Specific Camera Traits](../engines/) - Camera implementations for different game engines
- [View Angles Documentation](../trigonometry/view_angles.md) - Understanding pitch/yaw/roll
- [Getting Started Guide](../getting_started.md) - Quick start with projection
- [Tutorials - World-to-Screen](../tutorials.md#tutorial-2-world-to-screen-projection) - Complete projection tutorial

---

*Last updated: 1 Nov 2025*
