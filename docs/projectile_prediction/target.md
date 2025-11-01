# `omath::projectile_prediction::Target` — Target state for aim solvers

> Header: `omath/projectile_prediction/target.hpp`
> Namespace: `omath::projectile_prediction`
> Used by: `ProjPredEngineInterface` implementations (e.g., Legacy/AVX2 engines)

A small POD-style container describing a target’s **current pose** and **motion** for projectile lead/aim computations.

---

## API

```cpp
namespace omath::projectile_prediction {

class Target final {
public:
  Vector3<float> m_origin;    // Current world-space position of the target
  Vector3<float> m_velocity;  // World-space linear velocity (units/sec)
  bool           m_is_airborne{}; // Domain hint (e.g., ignore ground snapping)
};

} // namespace omath::projectile_prediction
```

---

## Field semantics

* **`m_origin`** — target position in world coordinates (same units as your `Vector3<float>` grid).
* **`m_velocity`** — instantaneous linear velocity. Solvers commonly assume **constant velocity** between “now” and impact unless your trait injects gravity/accel.
* **`m_is_airborne`** — optional hint for engine/trait logic (e.g., apply gravity to the target, skip ground friction/snap). Exact meaning is engine-dependent.

> Keep units consistent with your projectile model (e.g., meters & seconds). If projectiles use `g = 9.81 m/s²`, velocity should be in m/s and positions in meters.

---

## Typical usage

```cpp
using namespace omath::projectile_prediction;

Target tgt;
tgt.m_origin    = { 42.0f, 1.8f, -7.5f };
tgt.m_velocity  = {  3.0f, 0.0f,  0.0f }; // moving +X at 3 units/s
tgt.m_is_airborne = false;

// Feed into an aim solver with a Projectile
auto aim = engine->maybe_calculate_aim_point(projectile, tgt);
```

---

## Notes & tips

* If you track acceleration (e.g., gravity on ragdolls), your **EngineTrait** may derive it from `m_is_airborne` and world gravity; otherwise most solvers treat the target’s motion as linear.
* For highly agile targets, refresh `m_origin`/`m_velocity` every tick and re-solve; don’t reuse stale aim points.
* Precision: `Vector3<float>` is typically enough; if you need sub-millimeter accuracy over long ranges, consider double-precision internally in your trait.

---

## See also

* `Projectile` — shooter origin, muzzle speed, gravity scale
* `ProjPredEngineInterface` — common interface for aim solvers
* `ProjPredEngineLegacy`, `ProjPredEngineAvx2` — concrete solvers using this data

---

*Last updated: 1 Nov 2025*
