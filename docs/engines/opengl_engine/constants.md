# `omath::opengl_engine` — types & constants

> Header: `omath/engines/opengl_engine/constants.hpp`
> Namespace: `omath::opengl_engine`
> Purpose: define OpenGL coordinate system, matrix types, and angle ranges

---

## Summary

The **OpenGL Engine** uses a **Y-up, right-handed** coordinate system:

* **Up** = `{0, 1, 0}` (Y-axis)
* **Right** = `{1, 0, 0}` (X-axis)
* **Forward** = `{0, 0, -1}` (negative Z-axis)

Matrices are **column-major**. Angles are **clamped pitch** (±90°) and **normalized yaw/roll** (±180°).

---

## Constants

```cpp
namespace omath::opengl_engine {
  constexpr Vector3<float> k_abs_up      = {0, 1, 0};
  constexpr Vector3<float> k_abs_right   = {1, 0, 0};
  constexpr Vector3<float> k_abs_forward = {0, 0, -1};
}
```

These basis vectors define the engine's **world coordinate frame**.

---

## Matrix types

```cpp
using Mat4X4 = Mat<4, 4, float, MatStoreType::COLUMN_MAJOR>;
using Mat3X3 = Mat<4, 4, float, MatStoreType::COLUMN_MAJOR>;
using Mat1X3 = Mat<1, 3, float, MatStoreType::COLUMN_MAJOR>;
```

**Column-major** storage means columns are contiguous in memory. This matches OpenGL's native matrix layout and shader expectations (GLSL).

---

## Angle types

```cpp
using PitchAngle = Angle<float, -90.f, 90.f, AngleFlags::Clamped>;
using YawAngle   = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;
using RollAngle  = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;

using ViewAngles = omath::ViewAngles<PitchAngle, YawAngle, RollAngle>;
```

* **PitchAngle**: clamped to **[-90°, +90°]** (looking down vs. up)
* **YawAngle**: normalized to **[-180°, +180°]** (horizontal rotation)
* **RollAngle**: normalized to **[-180°, +180°]** (camera roll)

`ViewAngles` bundles all three into a single type for camera/view transforms.

---

## Coordinate system notes

* **Y-up**: gravity points along `-Y`, height increases with `+Y`
* **Right-handed**: cross product `right × up = forward` (forward is `-Z`)
* **Forward = -Z**: the camera looks down the negative Z-axis (OpenGL convention)
* This matches **OpenGL** conventions for 3D graphics pipelines

---

## See also

* `omath/engines/opengl_engine/formulas.hpp` — view/projection matrix builders
* `omath/trigonometry/angle.hpp` — angle normalization & clamping helpers
* `omath/trigonometry/view_angles.hpp` — generic pitch/yaw/roll wrapper
