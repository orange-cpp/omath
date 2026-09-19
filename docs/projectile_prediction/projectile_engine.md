# `omath::projectile_prediction::ProjPredEngineInterface` — Aim solver interface

> Header: `omath/projectile_prediction/proj_pred_engine.hpp`
> Namespace: `omath::projectile_prediction`
> Template: `ProjPredEngineInterface<ArithmeticType = float>`
> Depends on: `Vector3`, `Projectile`, `Launcher`, `Target`
> Purpose: **contract** for engines that compute a lead/aim solution to hit a moving target.
> Implementations: `ProjPredEngineLegacy`, `ProjPredEngineAvx2`, `ProjPredEngineDrag`

---

## Overview

An engine solves one question: which **view angles** must the eye have so that a projectile leaving the muzzle described by a [`Launcher`](launcher.md) intersects the [`Target`](target.md) under the engine’s physical model. The answer comes back as an `AimSolution`: the angles, a point on the eye's aim ray for drawing, the predicted target position and the time of flight.

If no intercept exists the engine returns `std::nullopt`.

---

## API

```cpp
namespace omath::projectile_prediction {

// View angles in degrees. Pitch is positive upwards whatever the engine's own sign convention is.
template<class T = float>
struct AimAngles { T pitch{}; T yaw{}; };

template<class T = float>
struct AimSolution {
  AimAngles<T> angles;                  // set these on the eye; launch pitch = angles.pitch + launcher.launch_pitch_offset
  Vector3<T>   aim_point;               // on the eye's aim ray, as far away as the predicted target
  Vector3<T>   predicted_target_position;
  T            time_of_flight{};
};

template<class T = float>
class ProjPredEngineInterface {
public:
  [[nodiscard]]
  virtual std::optional<AimSolution<T>>
  maybe_calculate_aim(const Projectile<T>& projectile, const Launcher<T>& launcher, const Target<T>& target) const = 0;

  // Compatibility wrappers over maybe_calculate_aim(), kept for one release.
  // eye = projectile.m_origin, world_offset = projectile.m_launch_offset, no view-relative muzzle offset.
  [[nodiscard]] std::optional<Vector3<T>>    maybe_calculate_aim_point (const Projectile<T>&, const Target<T>&) const;
  [[nodiscard]] std::optional<AimAngles<T>>  maybe_calculate_aim_angles(const Projectile<T>&, const Target<T>&) const;

  static constexpr Launcher<T> launcher_from_projectile(const Projectile<T>& projectile) noexcept;

  virtual ~ProjPredEngineInterface() = default;

protected:
  // copy and move are protected: no slicing through a base reference, derived engines stay copyable
};

} // namespace omath::projectile_prediction
```

### Semantics

* **Input**

    * `Projectile` — launch speed and gravity scale.
    * `Launcher` — eye origin, view-relative muzzle offset, optional fixed world offset, optional launch pitch offset.
    * `Target` — position, velocity, airborne flag.

* **Output**

    * `angles` — the camera angles that make the shot connect. The round leaves along them, raised by `Launcher::launch_pitch_offset` for weapons that fire above the crosshair.
    * `aim_point` — `eye_origin + forward(angles) * distance(eye, predicted target)`. Any point on that ray projects to the same pixel, so `world_to_screen(aim_point)` is where the crosshair must go. The angles and the point can never disagree.
    * `std::nullopt` — no solution (target outruns the projectile, out of range, degenerate input).

* **No side effects**: methods are `const` and do not modify inputs.

### Wrappers

`maybe_calculate_aim_point` and `maybe_calculate_aim_angles` exist so code written against the previous interface keeps compiling. They build a `Launcher` from the projectile's `m_origin` and `m_launch_offset` and forward to `maybe_calculate_aim`. Each call runs the full solve, so callers wanting both should call `maybe_calculate_aim` once.

---

## Typical usage

```cpp
using namespace omath::projectile_prediction;

std::unique_ptr<ProjPredEngineInterface<float>> engine = /* your implementation */;

constexpr Projectile<float> proj{.m_launch_speed = 850.f, .m_gravity_scale = 1.f};
constexpr Launcher<float> launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = {.forward = 16, .right = 8, .up = -6}};
Target<float> tgt = /* read from tracking system */;

if (const auto aim = engine->maybe_calculate_aim(proj, launcher, tgt))
{
    set_view_angles(aim->angles.pitch, aim->angles.yaw);   // or draw aim->aim_point
}
else
{
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

**Muzzle placement:** the launch origin depends on the angles being solved. Place the muzzle from the direct angles to the target, solve, then re-solve once from `launcher.launch_origin(trait.calc_view_basis(pitch, yaw))`. One refinement is enough for offsets of a few units against targets hundreds away.

**Robustness tips:**

* **Feasibility checks:** return `nullopt` when projectile speed ≤ 0, the target recedes faster than the projectile, or the solution time falls outside `[0, t_max]`.
* **Bounds:** clamp search time to `[0, max_flight_time]` or by range.
* **Tolerances:** use epsilons for convergence (e.g., `|f(t)| < 1e-4`, `|Δt| < 1e-4 s`).
* **Determinism:** compute times as `step * index` rather than accumulating, so runs are replayable.

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
* **Muzzle offset**: fire from `launcher.launch_origin(basis(angles))` with the returned angles and check the miss at `time_of_flight`.
* **Ray consistency**: camera angles towards `aim_point` equal `angles`.

---

## See Also

- [Launcher Documentation](launcher.md) - Eye origin and muzzle offset
- [Projectile Documentation](projectile.md) - Projectile properties
- [Target Documentation](target.md) - Target state representation
- [Legacy Implementation](proj_pred_engine_legacy.md) - Standard projectile prediction engine
- [AVX2 Implementation](proj_pred_engine_avx2.md) - Optimized AVX2 engine
- [Drag Implementation](proj_pred_engine_drag.md) - Engine for rounds with air drag
- [Tutorials - Projectile Prediction](../tutorials.md#tutorial-3-projectile-prediction-aim-bot) - Complete aim-bot tutorial

---

*Last updated: 18 Sep 2026*
