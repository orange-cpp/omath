# `omath::projection::Camera` — Generic, trait-driven camera with screen/world conversion

> Header: `omath/projection/camera.hpp`
> Namespace: `omath::projection`
> Template: `Camera<Mat4X4Type, ViewAnglesType, TraitClass, depth_range, axes, NumericType>`
> Requires: `CameraEngineConcept<TraitClass, Mat4X4Type, ViewAnglesType, NumericType>`
> Key features: **lazy view / projection / view-projection / inverse caching**, world↔screen helpers, frustum culling, pluggable math via a **Trait**

---

## Overview

`Camera` is a small, zero-allocation camera wrapper. It delegates the math for **view**, **projection**, and **look-at** to a **Trait** (`TraitClass`), which lets you plug in different coordinate systems or conventions without changing the camera code. The class caches the view, projection, view-projection and inverse view-projection matrices and invalidates them when a parameter changes.

Every engine module ships a ready-made alias, e.g. `omath::source_engine::Camera`, `omath::unreal_engine::Camera`, `omath::opengl_engine::Camera`. Use those unless you are integrating a new engine.

Alongside the camera, the header defines:

* `class ViewPort { float m_width, m_height; float aspect_ratio() const noexcept; }`
* `using FieldOfView = Angle<float, 0.f, 180.f, AngleFlags::Clamped>;`
* `enum class ViewPortClipping { AUTO, MANUAL };`
* `struct CameraAxes { bool inverted_forward, inverted_right; };`

---

## Template parameters & trait requirements

```cpp
template<class Mat4X4Type, class ViewAnglesType, class TraitClass,
         NDCDepthRange depth_range = NDCDepthRange::NEGATIVE_ONE_TO_ONE,
         CameraAxes    axes        = {},
         class         NumericType = float>
requires CameraEngineConcept<TraitClass, Mat4X4Type, ViewAnglesType, NumericType>
class Camera final;
```

| Parameter        | Meaning |
|------------------|---------|
| `Mat4X4Type`     | A `Mat<4,4,NumericType,...>`; supplies `*`, `inverted()`, `.at(r,c)`, `operator[r,c]`, `get_store_ordering()`. |
| `ViewAnglesType` | The engine's pitch/yaw/roll aggregate. |
| `TraitClass`     | Static functions that build the matrices (below). |
| `depth_range`    | `NEGATIVE_ONE_TO_ONE` (OpenGL) or `ZERO_TO_ONE` (DirectX / Vulkan / Source). Drives clipping and frustum planes. |
| `axes`           | Which basis rows of the view matrix point *away* from the engine's convention; used by `get_abs_*` and `calc_view_angles_from_view_matrix`. |
| `NumericType`    | `float` or `double`. All positions and planes use it; `ViewPort` and `FieldOfView` stay `float`. |

```cpp
template<class T, class MatType, class ViewAnglesType, class NumericType>
concept CameraEngineConcept = requires(
  const Vector3<NumericType>& cam_origin, const Vector3<NumericType>& look_at,
  const ViewAnglesType& angles, const FieldOfView& fov, const ViewPort& viewport,
  NumericType z_near, NumericType z_far, NDCDepthRange ndc_depth_range)
{
  { T::calc_look_at_angle(cam_origin, look_at) }                                   -> std::same_as<ViewAnglesType>;
  { T::calc_view_matrix(angles, cam_origin) }                                       -> std::same_as<MatType>;
  { T::calc_projection_matrix(fov, viewport, z_near, z_far, ndc_depth_range) }     -> std::same_as<MatType>;
  requires std::is_floating_point_v<NumericType>;
  // all three must be noexcept
};
```

---

## Quick start

```cpp
using namespace omath;

projection::ViewPort vp{1920.f, 1080.f};
constexpr auto fov = projection::FieldOfView::from_degrees(90.f);

source_engine::Camera cam(/*position*/ {0.f, 0.f, 64.f},
                          /*angles*/   source_engine::ViewAngles{},
                          vp, fov,
                          /*near*/     0.01f,
                          /*far*/      10000.f);

cam.look_at({512.f, 96.f, 48.f});

// World → screen pixels (origin top-left by default)
if (const auto s = cam.world_to_screen({600.f, 100.f, 40.f}))
{
    // s->x, s->y in pixels; s->z is NDC depth
}

// Same, but only rejects points behind the camera or outside the depth range
const auto off_screen_ok = cam.world_to_screen_unclipped<source_engine::Camera::ScreenStart::BOTTOM_LEFT_CORNER>({...});

// Screen → world ray end (z = 1 → far plane)
const auto far_point = cam.screen_to_world(Vector2<float>{960.f, 540.f});

// Culling
const bool hidden = cam.is_aabb_culled_by_frustum(primitives::Aabb<float>{{0,0,0}, {10,10,10}});
```

---

## API

```cpp
class Camera final {
public:
  enum class ScreenStart { TOP_LEFT_CORNER, BOTTOM_LEFT_CORNER };
  struct ProjectionParams { FieldOfView fov; NumericType aspect_ratio; };

  constexpr Camera(const Vector3<NumericType>& position, const ViewAnglesType& view_angles,
                   const ViewPort& view_port, const FieldOfView& fov,
                   NumericType near, NumericType far) noexcept;

  // --- Static helpers for matrices captured from a running engine ---
  static constexpr ProjectionParams extract_projection_params(const Mat4X4Type& proj) noexcept;
  static constexpr ViewAnglesType   calc_view_angles_from_view_matrix(const Mat4X4Type& view) noexcept;
  static constexpr Vector3<NumericType> calc_origin_from_view_matrix(const Mat4X4Type& view) noexcept;

  // --- Orientation ---
  constexpr void           look_at(const Vector3<NumericType>& target) noexcept;   // sets angles, invalidates view
  constexpr ViewAnglesType calc_look_at_angles(const Vector3<NumericType>& look_to) const noexcept;

  constexpr Vector3<NumericType> get_forward() const noexcept;   // rows of the view matrix's rotation block
  constexpr Vector3<NumericType> get_right()   const noexcept;
  constexpr Vector3<NumericType> get_up()      const noexcept;
  constexpr Vector3<NumericType> get_abs_forward() const noexcept;  // sign-corrected by `axes`
  constexpr Vector3<NumericType> get_abs_right()   const noexcept;
  constexpr Vector3<NumericType> get_abs_up()      const noexcept;

  // --- Matrices: calc_* always recompute, get_* are cached ---
  constexpr Mat4X4Type calc_view_matrix()            const noexcept;
  constexpr Mat4X4Type calc_projection_matrix()      const noexcept;
  constexpr Mat4X4Type calc_view_projection_matrix() const noexcept;
  constexpr const Mat4X4Type& get_view_matrix()            const noexcept;
  constexpr const Mat4X4Type& get_projection_matrix()      const noexcept;
  constexpr const Mat4X4Type& get_view_projection_matrix() const noexcept;
  constexpr const std::optional<Mat4X4Type>& get_inv_view_projection_matrix() const noexcept; // empty if singular

  // --- Setters (each invalidates the affected caches) ---
  constexpr void set_field_of_view(const FieldOfView&) noexcept;
  constexpr void set_near_plane(NumericType) noexcept;
  constexpr void set_far_plane(NumericType) noexcept;
  constexpr void set_view_port(const ViewPort&) noexcept;
  constexpr void set_view_angles(const ViewAnglesType&) noexcept;
  constexpr void set_origin(const Vector3<NumericType>&) noexcept;

  // --- Getters ---
  constexpr const FieldOfView&          get_field_of_view() const noexcept;
  constexpr const NumericType&          get_near_plane()    const noexcept;
  constexpr const NumericType&          get_far_plane()     const noexcept;
  constexpr const ViewAnglesType&       get_view_angles()   const noexcept;
  constexpr const Vector3<NumericType>& get_origin()        const noexcept;

  // --- Projection ---
  template<ScreenStart = ScreenStart::TOP_LEFT_CORNER>
  constexpr std::expected<Vector3<NumericType>, Error> world_to_screen(const Vector3<NumericType>&) const noexcept;
  template<ScreenStart = ScreenStart::TOP_LEFT_CORNER>
  constexpr std::expected<Vector3<NumericType>, Error> world_to_screen_unclipped(const Vector3<NumericType>&) const noexcept;

  constexpr std::expected<Vector3<NumericType>, Error>
  world_to_view_port(const Vector3<NumericType>& world, ViewPortClipping = ViewPortClipping::AUTO) const noexcept; // → NDC
  constexpr Vector3<NumericType> world_to_view_coordinates(const Vector3<NumericType>& world) const noexcept;        // → view space

  // --- Unprojection ---
  constexpr std::expected<Vector3<NumericType>, Error> view_port_to_world(const Vector3<NumericType>& ndc) const noexcept;
  template<ScreenStart = ScreenStart::TOP_LEFT_CORNER>
  constexpr std::expected<Vector3<NumericType>, Error> screen_to_world(const Vector3<NumericType>& screen) const noexcept;
  template<ScreenStart = ScreenStart::TOP_LEFT_CORNER>
  constexpr std::expected<Vector3<NumericType>, Error> screen_to_world(const Vector2<NumericType>& screen) const noexcept; // z = 1

  // --- NDC ↔ screen (pure arithmetic, no matrices) ---
  constexpr Vector3<NumericType> ndc_to_screen_position_from_top_left_corner(const Vector3<NumericType>& ndc) const noexcept;
  constexpr Vector3<NumericType> ndc_to_screen_position_from_bottom_left_corner(const Vector3<NumericType>& ndc) const noexcept;
  template<ScreenStart = ScreenStart::TOP_LEFT_CORNER>
  constexpr Vector3<NumericType> screen_to_ndc(const Vector3<NumericType>& screen) const noexcept;

  // --- Frustum culling (Gribb-Hartmann planes from the view-projection matrix) ---
  constexpr bool is_culled_by_frustum(const Triangle<Vector3<NumericType>>&) const noexcept;
  constexpr bool is_aabb_culled_by_frustum(const primitives::Aabb<NumericType>&) const noexcept;
  constexpr bool is_obb_culled_by_frustum(const primitives::Obb<NumericType>&) const noexcept;
};
```

### Error handling

All conversions return `std::expected<Vector3<NumericType>, Error>` with errors from `error_codes.hpp`:

| Error | Raised by | Meaning |
|---|---|---|
| `PERSPECTIVE_DIVIDER_LESS_EQ_ZERO` | `world_to_view_port`, `world_to_screen*`, `view_port_to_world`, `screen_to_world` | Homogeneous `w` is not usable: at or behind the camera when projecting, or ~0 when unprojecting. |
| `WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS` | `world_to_view_port`, `world_to_screen*` | NDC outside the clip volume. `AUTO` checks x, y and z; `MANUAL` (the `_unclipped` variants) checks only z. |
| `INV_VIEW_PROJ_MAT_DET_EQ_ZERO` | `view_port_to_world`, `screen_to_world` | View-projection matrix is singular (degenerate camera). |

---

## Coordinate spaces & conversions

### World → NDC (`world_to_view_port`)

1. Take the cached `VP = P * V`.
2. Compute `VP * (x, y, z, 1)` as four dot products.
3. Reject if `w <= epsilon`.
4. Perspective divide → NDC. `x, y ∈ [-1,1]`; `z ∈ [-1,1]` or `[0,1]` depending on `depth_range`.
5. Reject if out of range (`AUTO`: all three axes, `MANUAL`: only z).

### NDC → Screen (pixels)

* **Top-left (default)**: `x_px = (x_ndc + 1)/2 * width`, `y_px = (-y_ndc/2 + 0.5) * height`
* **Bottom-left**: `x_px = (x_ndc + 1)/2 * width`, `y_px = (y_ndc/2 + 0.5) * height`

### Screen → NDC (`screen_to_ndc<ScreenStart>`)

Inverse of the above. `z` is passed through: use `0` for the near plane and `1` for the far plane when building a ray.

### NDC → World (`view_port_to_world`)

1. Take the cached `VP⁻¹`; if the matrix is singular → `INV_VIEW_PROJ_MAT_DET_EQ_ZERO`.
2. `VP⁻¹ * (x, y, z, 1)`, divide by `w` (→ `PERSPECTIVE_DIVIDER_LESS_EQ_ZERO` when `|w| < epsilon`).

> To build a **world-space ray** from a pixel, unproject at `z=0` (near) and `z=1` (far).

---

## Caching & invalidation

| Cache | Filled by | Dropped by |
|---|---|---|
| view matrix | `get_view_matrix` | `set_origin`, `set_view_angles`, `look_at` |
| projection matrix | `get_projection_matrix` | `set_field_of_view`, `set_near_plane`, `set_far_plane`, `set_view_port` |
| view-projection | `get_view_projection_matrix` | any of the above |
| inverse view-projection | `get_inv_view_projection_matrix` | whenever view-projection is rebuilt |

In constant-evaluated contexts (`constexpr` cameras) the caches are bypassed and every query recomputes.

---

## Notes & gotchas

* **Matrix order**: the camera multiplies `P * V`. Your **Trait** must match.
* **Store ordering**: `Mat4X4Type::get_store_ordering()` decides how `at(r, c)` indexes memory; both orderings are supported transparently.
* **`ViewPort` and `FieldOfView` are `float`** even for `double` cameras; `extract_projection_params` casts.
* **`world_to_view_port` returns NDC**, not pixels, despite the name.

---

## Minimal trait sketch (column-major, left-handed)

```cpp
struct LHCTrait {
  static MyAngles calc_look_at_angle(const Vector3<float>& eye, const Vector3<float>& at) noexcept { /* ... */ }

  static Mat4 calc_view_matrix(const MyAngles& ang, const Vector3<float>& eye) noexcept {
    // Build from forward/right/up and translation, e.g. via mat_camera_view()
  }

  static Mat4 calc_projection_matrix(const FieldOfView& fov, const ViewPort& vp,
                                     float zn, float zf, NDCDepthRange range) noexcept {
    return range == NDCDepthRange::ZERO_TO_ONE
      ? mat_perspective_left_handed_vertical_fov<float, MatStoreType::COLUMN_MAJOR, NDCDepthRange::ZERO_TO_ONE>(fov.as_degrees(), vp.aspect_ratio(), zn, zf)
      : mat_perspective_left_handed_vertical_fov<float, MatStoreType::COLUMN_MAJOR>(fov.as_degrees(), vp.aspect_ratio(), zn, zf);
  }
};

using MyCamera = projection::Camera<Mat4, MyAngles, LHCTrait, NDCDepthRange::ZERO_TO_ONE>;
```

---

## See Also

- [Engine-Specific Camera Traits](../engines/) - Camera implementations for different game engines
- [View Angles Documentation](../trigonometry/view_angles.md) - Understanding pitch/yaw/roll
- [Getting Started Guide](../getting_started.md) - Quick start with projection
- [Tutorials - World-to-Screen](../tutorials.md#tutorial-2-world-to-screen-projection) - Complete projection tutorial

---

*Last updated: 18 Sep 2026*
