# `omath::unity_engine` — types & constants

> Header: `omath/engines/unity_engine/constants.hpp`
> Namespace: `omath::unity_engine`
> Purpose: define Unity Engine coordinate system, matrix types, and angle ranges

---

## Summary

The **Unity Engine** uses a **Y-up, left-handed** coordinate system:

* **Up** = `{0, 1, 0}` (Y-axis)
* **Right** = `{1, 0, 0}` (X-axis)
* **Forward** = `{0, 0, 1}` (Z-axis)

Matrices are **row-major**. Angles are **clamped pitch** (±90°) and **normalized yaw/roll** (±180°).

---

## Constants

```cpp
namespace omath::unity_engine {
  constexpr Vector3<float> k_abs_up      = {0, 1, 0};
  constexpr Vector3<float> k_abs_right   = {1, 0, 0};
  constexpr Vector3<float> k_abs_forward = {0, 0, 1};
}
```

These basis vectors define the engine's **world coordinate frame**.

---

## Matrix types

```cpp
using Mat4X4 = Mat<4, 4, float, MatStoreType::ROW_MAJOR>;
using Mat3X3 = Mat<4, 4, float, MatStoreType::ROW_MAJOR>;
using Mat1X3 = Mat<1, 3, float, MatStoreType::ROW_MAJOR>;
```

**Row-major** storage means rows are contiguous in memory. Suitable for CPU-side transforms and typical C++ math libraries.

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
* **Left-handed**: cross product `forward × right = up` with left-hand rule
* This matches **Unity Engine** conventions for 3D games and simulations

---

## See also

* `omath/engines/unity_engine/formulas.hpp` — view/projection matrix builders
* `omath/trigonometry/angle.hpp` — angle normalization & clamping helpers
* `omath/trigonometry/view_angles.hpp` — generic pitch/yaw/roll wrapper
