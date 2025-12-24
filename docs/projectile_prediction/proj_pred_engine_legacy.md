# `omath::projectile_prediction::ProjPredEngineLegacy` — Legacy trait-based aim solver

> Header: `omath/projectile_prediction/proj_pred_engine_legacy.hpp`
> Namespace: `omath::projectile_prediction`
> Inherits: `ProjPredEngineInterface`
> Template param (default): `EngineTrait = source_engine::PredEngineTrait`
> Purpose: compute a world-space **aim point** to hit a (possibly moving) target using a **discrete time scan** and a **closed-form ballistic pitch** under constant gravity.

---

## Overview

`ProjPredEngineLegacy` is a portable, trait-driven projectile lead solver. At each simulation time step `t` it:

1. **Predicts target position** with `EngineTrait::predict_target_position(target, t, g)`.
2. **Computes launch pitch** via a gravity-aware closed form (or a direct angle if gravity is zero).
3. **Validates** that a projectile fired with that pitch (and direct yaw) actually reaches the predicted target within a **distance tolerance** at time `t`.
4. On success, **returns an aim point** computed by `EngineTrait::calc_viewpoint_from_angles(...)`.

If no time step yields a feasible solution up to `maximum_simulation_time`, returns `std::nullopt`.

---

## API

```cpp
template<class EngineTrait = source_engine::PredEngineTrait>
requires PredEngineConcept<EngineTrait>
class ProjPredEngineLegacy final : public ProjPredEngineInterface {
public:
  ProjPredEngineLegacy(float gravity_constant,
                       float simulation_time_step,
                       float maximum_simulation_time,
                       float distance_tolerance);

  [[nodiscard]]
  std::optional<Vector3<float>>
  maybe_calculate_aim_point(const Projectile& projectile,
                            const Target& target) const override;

private:
  // Closed-form ballistic pitch solver (internal)
  std::optional<float>
  maybe_calculate_projectile_launch_pitch_angle(const Projectile& projectile,
                                                const Vector3<float>& target_position) const noexcept;

  bool is_projectile_reached_target(const Vector3<float>& target_position,
                                    const Projectile& projectile,
                                    float pitch, float time) const noexcept;

  const float m_gravity_constant;
  const float m_simulation_time_step;
  const float m_maximum_simulation_time;
  const float m_distance_tolerance;
};
```

### Constructor parameters

* `gravity_constant` — magnitude of gravity (e.g., `9.81f`), world units/s².
* `simulation_time_step` — Δt for the scan (e.g., `1/240.f`).
* `maximum_simulation_time` — search horizon in seconds.
* `distance_tolerance` — max allowed miss distance at time `t` to accept a solution.

---

## Trait requirements (`PredEngineConcept`)

Your `EngineTrait` must expose **noexcept** static methods with these signatures:

```cpp
Vector3<float> predict_projectile_position(const Projectile&, float pitch_deg, float yaw_deg,
                                           float time, float gravity) noexcept;

Vector3<float> predict_target_position(const Target&, float time, float gravity) noexcept;

float          calc_vector_2d_distance(const Vector3<float>& v) noexcept;   // typically length in XZ plane
float          get_vector_height_coordinate(const Vector3<float>& v) noexcept; // typically Y

Vector3<float> calc_viewpoint_from_angles(const Projectile&, Vector3<float> target,
                                          std::optional<float> maybe_pitch_deg) noexcept;

float          calc_direct_pitch_angle(const Vector3<float>& from, const Vector3<float>& to) noexcept;
float          calc_direct_yaw_angle  (const Vector3<float>& from, const Vector3<float>& to) noexcept;
```

> This design lets you adapt different game/physics conventions (axes, units, handedness) without changing the solver.

---

## Algorithm details

### Time scan

For `t = 0 .. maximum_simulation_time` in steps of `simulation_time_step`:

1. `T = EngineTrait::predict_target_position(target, t, g)`
2. `pitch = maybe_calculate_projectile_launch_pitch_angle(projectile, T)`

    * If `std::nullopt`: continue
3. `yaw = EngineTrait::calc_direct_yaw_angle(projectile.m_origin, T)`
4. `P = EngineTrait::predict_projectile_position(projectile, pitch, yaw, t, g)`
5. Accept if `|P - T| <= distance_tolerance`
6. Return `EngineTrait::calc_viewpoint_from_angles(projectile, T, pitch)`

### Closed-form pitch (gravity on)

Implements the classic ballistic formula (low-arc branch), where:

* `v` = muzzle speed,
* `g` = `gravity_constant * projectile.m_gravity_scale`,
* `x` = horizontal (2D) distance to target,
* `y` = vertical offset to target.

[
\theta ;=; \arctan!\left(\frac{v^{2} ;-; \sqrt{v^{4}-g!\left(gx^{2}+2yv^{2}\right)}}{gx}\right)
]

* If the **discriminant** ( v^{4}-g(gx^{2}+2yv^{2}) < 0 ) ⇒ **no real solution**.
* If `g == 0`, falls back to `EngineTrait::calc_direct_pitch_angle(...)`.
* Returns **degrees** (internally converts from radians).

---

## Usage example

```cpp
using namespace omath::projectile_prediction;

ProjPredEngineLegacy solver(
  /*gravity*/ 9.81f,
  /*dt*/      1.f / 240.f,
  /*Tmax*/    3.0f,
  /*tol*/     0.05f
);

Projectile proj; // fill: m_origin, m_launch_speed, m_gravity_scale, etc.
Target     tgt;  // fill: position/velocity as required by your trait

if (auto aim = solver.maybe_calculate_aim_point(proj, tgt)) {
  // Drive your turret/reticle toward *aim
} else {
  // No feasible intercept in the given horizon
}
```

---

## Behavior & edge cases

* **Zero gravity or zero distance**: uses direct pitch toward the target.
* **Negative discriminant** in the pitch formula: returns `std::nullopt` for that time step.
* **Very small `x`** (horizontal distance): the formula’s denominator `gx` approaches zero; your trait’s direct pitch helper provides a stable fallback.
* **Tolerance**: `distance_tolerance` controls acceptance; tighten for accuracy, loosen for robustness.

---

## Complexity & tuning

* Time: **O(T)** where ( T \approx \frac{\text{maximum_simulation_time}}{\text{simulation_time_step}} )
  plus trait costs for prediction and angle math per step.
* Smaller `simulation_time_step` improves precision but increases runtime.
* If needed, do a **coarse-to-fine** search: coarse Δt scan, then refine around the best hit time.

---

## Testing checklist

* Stationary, level target → pitch ≈ 0 for short ranges; accepted within tolerance.
* Elevated/depressed targets → pitch positive/negative as expected.
* Receding fast target → unsolved within horizon ⇒ `nullopt`.
* Gravity scale = 0 → identical to straight-line solution.
* Near-horizon shots (large range, small arc) → discriminant near zero; verify stability.

---

## Notes

* All angles produced/consumed by the trait in this implementation are **degrees**.
* `calc_viewpoint_from_angles` defines what “aim point” means in your engine (e.g., a point along the initial ray or the predicted impact point). Keep this consistent with your HUD/reticle.

---

*Last updated: 24 Dec 2025*
