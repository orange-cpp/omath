# `omath::projectile_prediction::ProjPredEngineInterface` — Aim-point solver interface

> Header: your project’s `projectile_prediction/proj_pred_engine_interface.hpp`
> Namespace: `omath::projectile_prediction`
> Depends on: `Vector3<float>`, `Projectile`, `Target`
> Purpose: **contract** for engines that compute a lead/aim point to hit a moving target.

---

## Overview

`ProjPredEngineInterface` defines a single pure-virtual method that attempts to compute the **world-space aim point** where a projectile should be launched to intersect a target under the engine’s physical model (e.g., constant projectile speed, gravity, drag, max flight time, etc.).

If a valid solution exists, the engine returns the 3D aim point. Otherwise, it returns `std::nullopt` (no feasible intercept).

---

## API

```cpp
namespace omath::projectile_prediction {

class ProjPredEngineInterface {
public:
  [[nodiscard]]
  virtual std::optional<Vector3<float>>
  maybe_calculate_aim_point(const Projectile& projectile,
                            const Target& target) const = 0;

  virtual ~ProjPredEngineInterface() = default;
};

} // namespace omath::projectile_prediction
```

### Semantics

* **Input**

    * `Projectile` — engine-specific projectile properties (typical: muzzle speed, gravity vector, drag flag/coeff, max range / flight time).
    * `Target` — target state (typical: position, velocity, possibly acceleration).

* **Output**

    * `std::optional<Vector3<float>>`

        * `value()` — world-space point to aim at **now** so that the projectile intersects the target under the model.
        * `std::nullopt` — no solution (e.g., target outruns projectile, blocked by constraints, numerical failure).

* **No side effects**: method is `const` and should not modify inputs.

---

## Typical usage

```cpp
using namespace omath::projectile_prediction;

std::unique_ptr<ProjPredEngineInterface> engine = /* your implementation */;
Projectile proj = /* fill from weapon config */;
Target     tgt  = /* read from tracking system */;

if (auto aim = engine->maybe_calculate_aim_point(proj, tgt)) {
  // Rotate/steer to (*aim)
} else {
  // Fall back: no-lead, predictive UI, or do not fire
}
```

---

## Implementation guidance (for engine authors)

**Common models:**

1. **No gravity, constant speed**
   Closed form intersect time `t` solves `‖p_t + v_t t − p_0‖ = v_p t`.
   Choose the smallest non-negative real root; aim point = `p_t + v_t t`.

2. **Gravity (constant g), constant speed**
   Solve ballistics with vertical drop: either numerical (Newton–Raphson on time) or 2D elevation + azimuth decomposition. Ensure convergence caps and time bounds.

3. **Drag**
   Typically requires numeric integration (e.g., RK4) wrapped in a root find on time-of-flight.

**Robustness tips:**

* **Feasibility checks:** return `nullopt` when:

    * projectile speed ≤ 0; target too fast in receding direction; solution time outside `[0, t_max]`.
* **Bounds:** clamp search time to reasonable `[t_min, t_max]` (e.g., `[0, max_flight_time]` or by range).
* **Tolerances:** use epsilons for convergence (e.g., `|f(t)| < 1e-4`, `|Δt| < 1e-4 s`).
* **Determinism:** fix iteration counts or seeds if needed for replayability.

---

## Example: constant-speed, no-gravity intercept (closed form)

```cpp
// Solve ||p + v t|| = s t  where p = target_pos - shooter_pos, v = target_vel, s = projectile_speed
// Quadratic: (v·v - s^2) t^2 + 2 (p·v) t + (p·p) = 0
inline std::optional<float> intercept_time_no_gravity(const Vector3<float>& p,
                                                      const Vector3<float>& v,
                                                      float s) {
  const float a = v.dot(v) - s*s;
  const float b = 2.f * p.dot(v);
  const float c = p.dot(p);
  if (std::abs(a) < 1e-6f) {                 // near linear
    if (std::abs(b) < 1e-6f) return std::nullopt;
    float t = -c / b;
    return t >= 0.f ? std::optional{t} : std::nullopt;
  }
  const float disc = b*b - 4.f*a*c;
  if (disc < 0.f) return std::nullopt;
  const float sqrtD = std::sqrt(disc);
  float t1 = (-b - sqrtD) / (2.f*a);
  float t2 = (-b + sqrtD) / (2.f*a);
  float t  = (t1 >= 0.f ? t1 : t2);
  return t >= 0.f ? std::optional{t} : std::nullopt;
}
```

Aim point (given shooter origin `S`, target pos `T`, vel `V`):

```
p = T - S
t* = intercept_time_no_gravity(p, V, speed)
aim = T + V * t*
```

Return `nullopt` if `t*` is absent.

---

## Testing checklist

* **Stationary target**: aim point equals target position when `s > 0`.
* **Target perpendicular motion**: lead equals lateral displacement `V⊥ * t`.
* **Receding too fast**: expect `nullopt`.
* **Gravity model**: verify arc solutions exist for short & long trajectories (if implemented).
* **Numerics**: convergence within max iterations; monotonic improvement of residuals.

---

## Notes

* This is an **interface** only; concrete engines (e.g., `SimpleNoGravityEngine`, `BallisticGravityEngine`) should document their assumptions (gravity, drag, wind, bounds) and units (meters, seconds).
* The coordinate system and handedness should be consistent with `Vector3<float>` and the rest of your math stack.

---

*Last updated: 1 Nov 2025*
