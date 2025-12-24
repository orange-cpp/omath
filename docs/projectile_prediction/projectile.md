# `omath::projectile_prediction::Projectile` — Projectile parameters for aim solvers

> Header: `omath/projectile_prediction/projectile.hpp`
> Namespace: `omath::projectile_prediction`
> Used by: `ProjPredEngineInterface` implementations (e.g., `ProjPredEngineLegacy`, `ProjPredEngineAvx2`)

`Projectile` is a tiny data holder that describes how a projectile is launched: **origin** (world position), **launch speed**, and a **gravity scale** (multiplier applied to the engine’s gravity constant).

---

## API

```cpp
namespace omath::projectile_prediction {

class Projectile final {
public:
  Vector3<float> m_origin;     // Launch position (world space)
  float          m_launch_speed{};   // Initial speed magnitude (units/sec)
  float          m_gravity_scale{};  // Multiplier for global gravity (dimensionless)
};

} // namespace omath::projectile_prediction
```

---

## Field semantics

* **`m_origin`**
  World-space position where the projectile is spawned (e.g., muzzle or emitter point).

* **`m_launch_speed`**
  Initial speed **magnitude** in your world units per second. Direction is determined by the solver (from yaw/pitch).

    * Must be **non-negative**. Zero disables meaningful ballistic solutions.

* **`m_gravity_scale`**
  Multiplies the engine’s gravity constant provided to the solver (e.g., `g = gravity_constant * m_gravity_scale`).

    * Use `1.0f` for normal gravity, `0.0f` for no-drop projectiles, other values to simulate heavier/lighter rounds.

> Units must be consistent across your project (e.g., meters & seconds). If `gravity_constant = 9.81f`, then `m_launch_speed` is in m/s and positions are in meters.

---

## Typical usage

```cpp
using namespace omath::projectile_prediction;

Projectile proj;
proj.m_origin        = { 0.0f, 1.6f, 0.0f }; // player eye / muzzle height
proj.m_launch_speed  = 850.0f;               // e.g., 850 m/s
proj.m_gravity_scale = 1.0f;                 // normal gravity

// With an aim solver:
auto aim = engine->maybe_calculate_aim_point(proj, target);
if (aim) {
  // rotate/aim toward *aim and fire
}
```

---

## With gravity-aware solver (outline)

Engines typically compute the firing angles to reach a predicted target position:

* Horizontal distance `x` and vertical offset `y` are derived from `target - m_origin`.
* Gravity used is `g = gravity_constant * m_gravity_scale`.
* Launch direction has speed `m_launch_speed` and angles solved by the engine.

If `m_gravity_scale == 0`, engines usually fall back to straight-line (no-drop) solutions.

---

## Validation & tips

* Keep `m_launch_speed ≥ 0`. Negative values are nonsensical.
* If your weapon can vary muzzle speed (charge-up, attachments), update `m_launch_speed` per shot.
* For different ammo types (tracers, grenades), prefer tweaking **`m_gravity_scale`** (and possibly the engine’s gravity constant) to match observed arc.

---

## See also

* `ProjPredEngineInterface` — common interface for aim solvers
* `ProjPredEngineLegacy` — trait-based, time-stepped ballistic solver
* `ProjPredEngineAvx2` — AVX2-accelerated solver with fixed-time pitch solve
* `Target` — target state consumed by the solvers
* `Vector3<float>` — math type used for positions and directions

---

*Last updated: 24 Dec 2025*
