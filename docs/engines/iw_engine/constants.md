# `omath::iw_engine` — types & constants

> Header: `omath/engines/iw_engine/constants.hpp`
> Namespace: `omath::iw_engine`
> Purpose: define IW Engine (Call of Duty) coordinate system, matrix types, and angle ranges

---

## Summary

The **IW Engine** (Infinity Ward Engine, used in Call of Duty series) uses a **Z-up, right-handed** coordinate system:

* **Up** = `{0, 0, 1}` (Z-axis)
* **Right** = `{0, -1, 0}` (negative Y-axis)
* **Forward** = `{1, 0, 0}` (X-axis)

Matrices are **row-major**. Angles are **clamped pitch** (±89°) and **normalized yaw/roll** (±180°).

---

## Constants

```cpp
namespace omath::iw_engine {
  constexpr Vector3<float> k_abs_up      = {0, 0, 1};
  constexpr Vector3<float> k_abs_right   = {0, -1, 0};
  constexpr Vector3<float> k_abs_forward = {1, 0, 0};
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
using PitchAngle = Angle<float, -89.f, 89.f, AngleFlags::Clamped>;
using YawAngle   = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;
using RollAngle  = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;

using ViewAngles = omath::ViewAngles<PitchAngle, YawAngle, RollAngle>;
```

* **PitchAngle**: clamped to **[-89°, +89°]** (looking down vs. up)
* **YawAngle**: normalized to **[-180°, +180°]** (horizontal rotation)
* **RollAngle**: normalized to **[-180°, +180°]** (camera roll)

`ViewAngles` bundles all three into a single type for camera/view transforms.

---

## Coordinate system notes

* **Z-up**: gravity points along `-Z`, height increases with `+Z`
* **Right-handed**: cross product `forward × right = up` holds
* This matches **IW Engine** (Call of Duty series: Modern Warfare, Black Ops, etc.) conventions

---

## See also

* `omath/engines/iw_engine/formulas.hpp` — view/projection matrix builders
* `omath/trigonometry/angle.hpp` — angle normalization & clamping helpers
* `omath/trigonometry/view_angles.hpp` — generic pitch/yaw/roll wrapper
