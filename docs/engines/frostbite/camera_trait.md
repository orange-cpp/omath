# `omath::frostbite_engine::CameraTrait` — plug-in trait for `projection::Camera`

> Header: `omath/engines/frostbite_engine/traits/camera_trait.hpp` • Impl: `omath/engines/frostbite_engine/traits/camera_trait.cpp`
> Namespace: `omath::frostbite_engine`
> Purpose: provide Frostbite-style **look-at**, **view**, and **projection** math to the generic `omath::projection::Camera` (satisfies `CameraEngineConcept`).

---

## Summary

`CameraTrait` exposes three `static` functions:

* `calc_look_at_angle(origin, look_at)` – computes Euler angles so the camera at `origin` looks at `look_at`. Implementation normalizes the direction, computes **pitch** as `-asin(dir.y)` and **yaw** as `atan2(dir.x, dir.z)`; **roll** is `0`. Pitch/yaw are returned using the project’s strong angle types (`PitchAngle`, `YawAngle`, `RollAngle`).
* `calc_view_matrix(angles, origin)` – delegates to Frostbite formulas `frostbite_engine::calc_view_matrix`, producing a `Mat4X4` view matrix for the given angles and origin.
* `calc_projection_matrix(fov, viewport, near, far)` – builds a perspective projection by calling `calc_perspective_projection_matrix(fov_degrees, aspect, near, far)`, where `aspect = viewport.aspect_ratio()`. Accepts `FieldOfView` (degrees).

The trait’s types (`ViewAngles`, `Mat4X4`, angle aliases) and helpers live in the Frostbite engine math headers included by the trait (`formulas.hpp`) and the shared projection header (`projection/camera.hpp`).

---

## API

```cpp
namespace omath::frostbite_engine {

class CameraTrait final {
public:
  // Compute Euler angles (pitch/yaw/roll) to look from cam_origin to look_at.
  static ViewAngles
  calc_look_at_angle(const Vector3<float>& cam_origin,
                     const Vector3<float>& look_at) noexcept;

  // Build view matrix for given angles and origin.
  static Mat4X4
  calc_view_matrix(const ViewAngles& angles,
                   const Vector3<float>& cam_origin) noexcept;

  // Build perspective projection from FOV (deg), viewport, near/far.
  static Mat4X4
  calc_projection_matrix(const projection::FieldOfView& fov,
                         const projection::ViewPort& view_port,
                         float near, float far) noexcept;
};

} // namespace omath::frostbite_engine
```

Uses: `Vector3<float>`, `ViewAngles` (pitch/yaw/roll), `Mat4X4`, `projection::FieldOfView`, `projection::ViewPort`.

---

## Behavior & conventions

* **Angles from look-at**:

  ```
  dir = normalize(look_at - origin)
  pitch = -asin(dir.y)     // +Y is up
  yaw   = atan2(dir.x, dir.z)
  roll  = 0
  ```

  Returned as `PitchAngle::from_radians(...)`, `YawAngle::from_radians(...)`, etc.

* **View matrix**: built by the Frostbite engine helper `frostbite_engine::calc_view_matrix(angles, origin)` to match the engine’s handedness and axis conventions.

* **Projection**: uses `calc_perspective_projection_matrix(fov.as_degrees(), viewport.aspect_ratio(), near, far)`. Pass your **vertical FOV** in degrees via `FieldOfView`; the helper computes a standard perspective matrix.

---

## Using with `projection::Camera`

Create a camera whose math is driven by this trait:

```cpp
using Mat4  = Mat4X4;                 // from Frostbite math headers
using Angs  = ViewAngles;             // pitch/yaw/roll type
using FBcam = omath::projection::Camera<Mat4, Angs, omath::frostbite_engine::CameraTrait>;

omath::projection::ViewPort vp{1920.f, 1080.f};
auto fov = omath::projection::FieldOfView::from_degrees(70.f);

FBcam cam(
  /*position*/   {0.f, 1.7f, -3.f},
  /*angles*/     omath::frostbite_engine::CameraTrait::calc_look_at_angle({0,1.7f,-3},{0,1.7f,0}),
  /*viewport*/   vp,
  /*fov*/        fov,
  /*near*/       0.1f,
  /*far*/        1000.f
);
```

This satisfies `CameraEngineConcept` expected by `projection::Camera` (look-at, view, projection) as declared in the trait header.

---

## Notes & tips

* Ensure your `ViewAngles` aliases (`PitchAngle`, `YawAngle`, `RollAngle`) match the project’s angle policy (ranges/normalization). The implementation constructs them **from radians**.
* `aspect_ratio()` is taken directly from `ViewPort` (`width / height`), so keep both positive and non-zero.
* `near` must be > 0 and `< far` for a valid projection matrix (enforced by your math helpers).

---

## See also

* Frostbite math helpers in `omath/engines/frostbite_engine/formulas.hpp` (view/projection builders used above).
* Generic camera wrapper `omath::projection::Camera` and its `CameraEngineConcept` (this trait is designed to plug straight into it). 
