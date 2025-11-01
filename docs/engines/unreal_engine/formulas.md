# `omath::unreal_engine` — formulas & matrix helpers

> Header: `omath/engines/unreal_engine/formulas.hpp`
> Namespace: `omath::unreal_engine`
> Purpose: compute direction vectors, rotation matrices, view matrices, and perspective projections for Unreal Engine

---

## Summary

This header provides **Unreal Engine**-specific math for:

* **Direction vectors** (`forward`, `right`, `up`) from `ViewAngles`
* **Rotation matrices** from Euler angles
* **View matrices** (camera transforms)
* **Perspective projection** matrices

All functions respect Unreal Engine's **Z-up, left-handed** coordinate system.

---

## API

```cpp
namespace omath::unreal_engine {

  // Compute forward direction from pitch/yaw/roll
  [[nodiscard]]
  Vector3<float> forward_vector(const ViewAngles& angles) noexcept;

  // Compute right direction from pitch/yaw/roll
  [[nodiscard]]
  Vector3<float> right_vector(const ViewAngles& angles) noexcept;

  // Compute up direction from pitch/yaw/roll
  [[nodiscard]]
  Vector3<float> up_vector(const ViewAngles& angles) noexcept;

  // Build 3x3 rotation matrix from angles
  [[nodiscard]]
  Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept;

  // Build view matrix (camera space transform)
  [[nodiscard]]
  Mat4X4 calc_view_matrix(const ViewAngles& angles,
                          const Vector3<float>& cam_origin) noexcept;

  // Build perspective projection matrix
  [[nodiscard]]
  Mat4X4 calc_perspective_projection_matrix(float field_of_view,
                                           float aspect_ratio,
                                           float near, float far) noexcept;

} // namespace omath::unreal_engine
```

---

## Direction vectors

Given camera angles (pitch/yaw/roll):

* `forward_vector(angles)` → unit vector pointing where the camera looks
* `right_vector(angles)` → unit vector pointing to the camera's right
* `up_vector(angles)` → unit vector pointing upward relative to the camera

These are used for movement, aim direction, and building coordinate frames.

---

## Rotation & view matrices

* `rotation_matrix(angles)` → 3×3 (or 4×4) rotation matrix from Euler angles
* `calc_view_matrix(angles, origin)` → camera view matrix

The view matrix transforms world coordinates into camera space (origin at camera, axes aligned with camera orientation).

---

## Perspective projection

```cpp
Mat4X4 proj = calc_perspective_projection_matrix(
  fov_degrees,    // vertical field of view (e.g., 90)
  aspect_ratio,   // width / height (e.g., 16/9)
  near_plane,     // e.g., 10.0
  far_plane       // e.g., 100000.0
);
```

Produces a **perspective projection matrix** suitable for 3D rendering pipelines. Combined with the view matrix, this implements the standard camera transform chain.

---

## Usage example

```cpp
using namespace omath::unreal_engine;

// Camera setup
ViewAngles angles = {
  PitchAngle::from_degrees(-20.0f),
  YawAngle::from_degrees(45.0f),
  RollAngle::from_degrees(0.0f)
};
Vector3<float> cam_pos{1000.0f, 500.0f, 200.0f};

// Compute direction
auto forward = forward_vector(angles);
auto right   = right_vector(angles);
auto up      = up_vector(angles);

// Build matrices
auto view_mat = calc_view_matrix(angles, cam_pos);
auto proj_mat = calc_perspective_projection_matrix(90.0f, 16.0f/9.0f, 10.0f, 100000.0f);

// Use view_mat and proj_mat for rendering...
```

---

## Conventions

* **Angles**: pitch (up/down), yaw (left/right), roll (tilt)
* **Pitch**: positive = looking up, negative = looking down
* **Yaw**: increases counter-clockwise from the +X axis
* **Coordinate system**: Z-up, X-forward, Y-right (left-handed)

---

## See also

* `omath/engines/unreal_engine/constants.hpp` — coordinate frame & angle types
* `omath/engines/unreal_engine/traits/camera_trait.hpp` — plug-in for generic `Camera`
* `omath/projection/camera.hpp` — generic camera wrapper using these formulas
