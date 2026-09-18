# `omath::projectile_prediction::ProjPredEngineLegacy` — Trait-based aim solver

> Header: `omath/projectile_prediction/proj_pred_engine_legacy.hpp`
> Namespace: `omath::projectile_prediction`
> Inherits: `ProjPredEngineInterface<ArithmeticType>`
> Template: `ProjPredEngineLegacy<EngineTrait = source_engine::PredEngineTrait, ArithmeticType = float>`
> Purpose: compute a world-space **aim point** or **aim angles** to hit a (possibly moving) target using a **fixed-step time scan** and a **closed-form ballistic pitch** under constant gravity.

> This class is kept for backward compatibility (`OMATH_ENABLE_LEGACY`). It is portable and trait-driven; `ProjPredEngineAvx2` is the vectorised alternative for `float` on x86.

---

## Overview

`ProjPredEngineLegacy` is a trait-driven projectile lead solver. For each time step `t = 0, Δt, 2Δt, …` below the horizon it:

1. **Predicts the target position** with `EngineTrait::predict_target_position(target, t, g)`.
2. **Computes the launch pitch** from the closed-form low-arc solution (or the direct angle when the projectile has no gravity, or when the target is straight above or below).
3. **Validates** that a projectile fired with that pitch and the direct yaw is within `distance_tolerance` of the predicted target position at time `t`.
4. On success, returns either the aim point from `EngineTrait::calc_viewpoint_from_angles(...)` or the `{pitch, yaw}` pair.

If no time step yields a feasible solution, both methods return `std::nullopt`.

---

## API

```cpp
template<class EngineTrait = source_engine::PredEngineTrait, class ArithmeticType = float>
requires PredEngineConcept<EngineTrait, ArithmeticType>
class ProjPredEngineLegacy final : public ProjPredEngineInterface<ArithmeticType> {
public:
  explicit ProjPredEngineLegacy(ArithmeticType gravity_constant,
                                ArithmeticType simulation_time_step,
                                ArithmeticType maximum_simulation_time,
                                ArithmeticType distance_tolerance) noexcept;

  // World-space point to look at (via EngineTrait::calc_viewpoint_from_angles)
  [[nodiscard]] std::optional<Vector3<ArithmeticType>>
  maybe_calculate_aim_point(const Projectile<ArithmeticType>&, const Target<ArithmeticType>&) const noexcept override;

  // Launch pitch and yaw in degrees, measured from projectile.m_origin + projectile.m_launch_offset
  [[nodiscard]] std::optional<AimAngles<ArithmeticType>>
  maybe_calculate_aim_angles(const Projectile<ArithmeticType>&, const Target<ArithmeticType>&) const noexcept override;
};

template<class ArithmeticType = float>
struct AimAngles { ArithmeticType pitch{}; ArithmeticType yaw{}; };
```

The engine is copyable and assignable, so it can live in containers or be swapped at runtime.

### Constructor parameters

* `gravity_constant` — magnitude of gravity in world units/s² (e.g. `800.f` for Source, `9.81f` for SI). Multiplied by `projectile.m_gravity_scale` for the projectile; applied unscaled to airborne targets.
* `simulation_time_step` — Δt of the scan. Must be positive.
* `maximum_simulation_time` — search horizon in seconds. Must be positive.
* `distance_tolerance` — maximum miss distance at time `t` to accept a solution.

A non-positive step or horizon makes both methods return `std::nullopt` immediately.

---

## Trait requirements (`PredEngineConcept<EngineTrait, ArithmeticType>`)

Your `EngineTrait` must expose **noexcept** static functions with these signatures (`T = ArithmeticType`):

```cpp
Vector3<T> predict_projectile_position(const Projectile<T>&, T pitch_deg, T yaw_deg, T time, T gravity) noexcept;
Vector3<T> predict_target_position(const Target<T>&, T time, T gravity) noexcept;

T          calc_vector_2d_distance(const Vector3<T>& v) noexcept;      // horizontal length, e.g. hypot(x, y)
T          get_vector_height_coordinate(const Vector3<T>& v) noexcept; // vertical component, e.g. z

Vector3<T> calc_viewpoint_from_angles(const Projectile<T>&, Vector3<T> predicted_target,
                                      std::optional<T> pitch_deg) noexcept;

T          calc_direct_pitch_angle(const Vector3<T>& from, const Vector3<T>& to) noexcept; // +89 up, -89 down
T          calc_direct_yaw_angle  (const Vector3<T>& from, const Vector3<T>& to) noexcept;
```

Ready-made traits exist for every supported engine, e.g. `source_engine::PredEngineTrait`, `unity_engine::PredEngineTrait`, `unreal_engine::PredEngineTrait`.

---

## Algorithm details

### Time scan

The scan runs `ceil(maximum_simulation_time / simulation_time_step)` steps with `t = simulation_time_step * i`. Computing `t` from the index rather than accumulating keeps the step count exact and avoids float drift over long horizons.

For each step:

1. `T = EngineTrait::predict_target_position(target, t, g)`
2. `pitch = launch_pitch(T)`; on `std::nullopt` continue
3. `yaw = EngineTrait::calc_direct_yaw_angle(launch_origin, T)`
4. `P = EngineTrait::predict_projectile_position(projectile, pitch, yaw, t, g)`
5. Accept if `|P - T|² <= distance_tolerance²`

where `launch_origin = projectile.m_origin + projectile.m_launch_offset`.

### Launch pitch

With `v` = launch speed, `g = gravity_constant * m_gravity_scale`, `x` = horizontal distance and `y` = height difference to the predicted target:

* `g == 0` → `EngineTrait::calc_direct_pitch_angle(launch_origin, T)`.
* Discriminant `D = v⁴ - g(gx² + 2yv²) < 0` → no real solution for this step.
* `x == 0` (straight up or down) → `calc_direct_pitch_angle`, i.e. ±90°.
* Otherwise the low-arc root

  ```
  tan θ = (v² - √D) / (g x)
        = (g x² + 2 y v²) / (x (v² + √D))       // conjugate form used in code
  ```

  The two are algebraically identical. The conjugate form avoids subtracting two nearly equal numbers, which for fast projectiles (thousands of units/s) costs about 0.002° in `float`; the conjugate form stays within 1e-5°.

Angles are returned in **degrees**.

---

## Usage example

```cpp
using namespace omath::projectile_prediction;

const ProjPredEngineLegacy<> solver(/*gravity*/ 800.f, /*dt*/ 1.f / 1000.f, /*horizon*/ 10.f, /*tolerance*/ 5.f);

constexpr Projectile<float> proj{.m_origin = {0, 0, 64}, .m_launch_speed = 3000.f, .m_gravity_scale = 0.5f};
constexpr Target<float> tgt{.m_origin = {900, 120, 0}, .m_velocity = {-40, 15, 0}, .m_is_airborne = false};

if (const auto aim = solver.maybe_calculate_aim_point(proj, tgt))
{
    // point the camera at *aim
}

if (const auto angles = solver.maybe_calculate_aim_angles(proj, tgt))
{
    // angles->pitch, angles->yaw in degrees
}
```

---

## Behavior & edge cases

* **Zero gravity** → straight-line solution via the direct pitch.
* **Straight up or down** → direct pitch (±90°); the reach check still has to pass.
* **Negative discriminant** → that step is skipped; if every step fails, `std::nullopt`.
* **Launch offset and yaw.** `maybe_calculate_aim_angles` measures yaw from the launch origin (`m_origin + m_launch_offset`). The Source trait's `calc_viewpoint_from_angles` builds the aim point relative to `m_origin`. With a purely forward offset both agree; with a lateral offset the yaw from the camera to the aim point differs from the reported yaw.
* **Tolerance** controls acceptance; tighten for accuracy, loosen for robustness. It is compared squared, so no square root runs per step.

---

## Complexity & tuning

* Time: **O(N)**, `N = ceil(maximum_simulation_time / simulation_time_step)`. Steps whose discriminant is negative cost a few flops; steps that reach the validation call the trait's projectile predictor, which dominates.
* Smaller `simulation_time_step` improves precision at linear cost.
* For many targets per frame at long range, prefer `ProjPredEngineAvx2` or a coarse-to-fine scan built on top of this engine.

---

## Testing checklist

* Stationary, level target → pitch ≈ 0 for short ranges; accepted within tolerance.
* Elevated/depressed targets → pitch positive/negative as expected.
* Receding fast target → unsolved within horizon ⇒ `nullopt`.
* Gravity scale = 0 → identical to straight-line solution.
* Straight above / below → ±90°.
* Non-positive step or horizon → `nullopt`, no hang.

---

*Last updated: 18 Sep 2026*
