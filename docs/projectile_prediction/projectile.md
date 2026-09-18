# `omath::projectile_prediction::Projectile` — Projectile parameters for aim solvers

> Header: `omath/projectile_prediction/projectile.hpp`
> Namespace: `omath::projectile_prediction`
> Template: `Projectile<ArithmeticType = float>`
> Used by: `ProjPredEngineInterface` implementations (`ProjPredEngineLegacy`, `ProjPredEngineAvx2`)

`Projectile` describes the round itself: **launch speed** and a **gravity scale** (multiplier applied to the engine’s gravity constant). Where it is fired from is described by [`Launcher`](launcher.md).

---

## API

```cpp
namespace omath::projectile_prediction {

template<class ArithmeticType = float>
class Projectile final {
public:
  // Read only by the compatibility wrappers of ProjPredEngineInterface (see below)
  Vector3<ArithmeticType> m_origin;
  Vector3<ArithmeticType> m_launch_offset{};

  ArithmeticType m_launch_speed{};   // Initial speed magnitude (units/sec)
  ArithmeticType m_gravity_scale{};  // Multiplier for the engine's gravity constant (dimensionless)
};

} // namespace omath::projectile_prediction
```

---

## Field semantics

* **`m_launch_speed`**
  Initial speed **magnitude** in your world units per second. Direction is determined by the solver.

    * Must be **non-negative**. Zero disables meaningful ballistic solutions.

* **`m_gravity_scale`**
  Multiplies the engine’s gravity constant (`g = gravity_constant * m_gravity_scale`).

    * Use `1.0f` for normal gravity, `0.0f` for no-drop projectiles, other values for heavier or lighter rounds.

* **`m_origin`, `m_launch_offset`** (compatibility only)
  The one-release compatibility wrappers `maybe_calculate_aim_point` and `maybe_calculate_aim_angles` read the eye from `m_origin` and treat `m_launch_offset` as a fixed world-space spawn offset. `maybe_calculate_aim` ignores both and takes a `Launcher` instead; new code leaves them default.

> Units must be consistent across your project (e.g., meters & seconds). If `gravity_constant = 9.81f`, then `m_launch_speed` is in m/s and positions are in meters.

---

## Typical usage

```cpp
using namespace omath::projectile_prediction;

constexpr Projectile<float> proj{.m_launch_speed = 850.f, .m_gravity_scale = 1.f};
constexpr Launcher<float> launcher{.eye_origin = {0.f, 1.6f, 0.f},
                                   .muzzle_offset = {.forward = 0.4f, .right = 0.1f, .up = -0.1f}};

if (const auto aim = engine->maybe_calculate_aim(proj, launcher, target))
{
    // set aim->angles on the camera, or draw aim->aim_point
}
```

---

## Validation & tips

* Keep `m_launch_speed ≥ 0`. Negative values are nonsensical.
* If your weapon can vary muzzle speed (charge-up, attachments), update `m_launch_speed` per shot.
* For different ammo types (tracers, grenades), tweak **`m_gravity_scale`** (and possibly the engine’s gravity constant) to match the observed arc.

---

## See also

* [`Launcher`](launcher.md) — eye origin and muzzle offset
* [`ProjPredEngineInterface`](projectile_engine.md) — common interface for aim solvers
* [`ProjPredEngineLegacy`](proj_pred_engine_legacy.md) — trait-based, time-stepped ballistic solver
* [`ProjPredEngineAvx2`](proj_pred_engine_avx2.md) — AVX2-accelerated solver
* [`Target`](target.md) — target state consumed by the solvers

---

*Last updated: 18 Sep 2026*
