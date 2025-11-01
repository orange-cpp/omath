# `omath::ViewAngles` — tiny POD for pitch/yaw/roll

> Header: your project’s `view_angles.hpp`
> Namespace: `omath`
> Kind: **aggregate struct** (POD), no methods, no allocation

A minimal container for Euler angles. You choose the types for each component (e.g., raw `float` or the strong `omath::Angle<>` type), and plug it into systems like `projection::Camera`.

---

## API

```cpp
namespace omath {
  template<class PitchType, class YawType, class RollType>
  struct ViewAngles {
    PitchType pitch;
    YawType   yaw;
    RollType  roll;
  };
}
```

* Aggregate: supports brace-init, aggregate copying, and `constexpr` usage when the component types do.
* Semantics (units/handedness/ranges) are **entirely defined by your chosen types**.

---

## Common aliases

```cpp
// Simple, raw degrees as floats (be careful with wrapping!)
using ViewAnglesF = omath::ViewAngles<float, float, float>;

// Safer, policy-based angles (recommended)
using PitchDeg = omath::Angle<float, -89.f,  89.f,  omath::AngleFlags::Clamped>;
using YawDeg   = omath::Angle<float, -180.f, 180.f, omath::AngleFlags::Normalized>;
using RollDeg  = omath::Angle<float, -180.f, 180.f, omath::AngleFlags::Normalized>;
using ViewAnglesDeg = omath::ViewAngles<PitchDeg, YawDeg, RollDeg>;
```

---

## Examples

### Basic construction

```cpp
omath::ViewAngles<float,float,float> a{ 10.f, 45.f, 0.f };   // pitch, yaw, roll in degrees
```

### With `omath::Angle<>` (automatic wrap/clamper)

```cpp
ViewAnglesDeg v{
  PitchDeg::from_degrees(  95.f), // ->  89deg (clamped)
  YawDeg::from_degrees   (-190.f), // -> 170deg (wrapped)
  RollDeg::from_degrees  (  30.f)
};
```

### Using with `projection::Camera`

```cpp
using Mat4 = omath::Mat<4,4,float, omath::MatStoreType::COLUMN_MAJOR>;
using Cam  = omath::projection::Camera<Mat4, ViewAnglesDeg, MyCameraTrait>;

omath::projection::ViewPort vp{1920,1080};
auto fov = omath::angles::degrees_to_radians(70.f); // or your Angle type

Cam cam(/*position*/ {0,1.7f,-3},
        /*angles*/   ViewAnglesDeg{ PitchDeg::from_degrees(0),
                                    YawDeg::from_degrees(0),
                                    RollDeg::from_degrees(0) },
        /*viewport*/ vp,
        /*fov*/      omath::Angle<float,0.f,180.f,omath::AngleFlags::Clamped>::from_degrees(70.f),
        /*near*/     0.1f,
        /*far*/      1000.f);
```

---

## Notes & tips

* **Ranges/units**: pick types that encode your policy (e.g., signed yaw in `[-180,180]`, pitch clamped to avoid gimbal flips).
* **Handedness & order**: this struct doesn’t impose rotation order. Your math/trait layer (e.g., `MyCameraTrait`) must define how `(pitch, yaw, roll)` map to a view matrix (common orders: ZYX or XYZ).
* **Zero-cost**: with plain `float`s this is as cheap as three scalars; with `Angle<>` you gain safety at the cost of tiny wrap/clamp logic on construction/arithmetic.
