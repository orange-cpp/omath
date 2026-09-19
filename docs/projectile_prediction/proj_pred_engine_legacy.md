# `omath::projectile_prediction::ProjPredEngineLegacy` — Trait-based aim solver

> Header: `omath/projectile_prediction/proj_pred_engine_legacy.hpp`
> Namespace: `omath::projectile_prediction`
> Inherits: `ProjPredEngineInterface<ArithmeticType>`
> Template: `ProjPredEngineLegacy<EngineTrait = source_engine::PredEngineTrait, ArithmeticType = float>`
> Purpose: solve the **view angles** (and a matching aim point) that make a projectile launched from a [`Launcher`](launcher.md) hit a (possibly moving) [`Target`](target.md), using a **fixed-step time scan** and a **closed-form ballistic pitch** under constant gravity.

> This class is kept under `OMATH_ENABLE_LEGACY`. It is portable and trait-driven; `ProjPredEngineAvx2` is the vectorised alternative for `float` on x86.

---

## Overview

For each time step `t = 0, Δt, 2Δt, …` below the horizon the engine:

1. **Predicts the target position** with `EngineTrait::predict_target_position(target, t, g)`.
2. **Places the muzzle** from the direct angles eye → target if the `Launcher` has a view-relative offset.
3. **Solves the launch pitch** from the closed-form low-arc solution (or the direct angle when the projectile has no gravity, or when the target is straight above or below), and the yaw as the bearing from the muzzle to the target.
4. **Re-places the muzzle** from the solved angles and solves once more (view-relative offset only).
5. **Validates** that a projectile fired from that muzzle with those angles is within `distance_tolerance` of the target at time `t`.
6. **Converts to view angles**: view pitch = launch pitch − `launcher.launch_pitch_offset`, rejecting the step if the engine would not let a player look there.
7. On success returns an `AimSolution`.

If no time step yields a feasible solution the result is `std::nullopt`.

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

  [[nodiscard]] std::optional<AimSolution<ArithmeticType>>
  maybe_calculate_aim(const Projectile<ArithmeticType>&, const Launcher<ArithmeticType>&,
                      const Target<ArithmeticType>&) const noexcept override;

  // maybe_calculate_aim_point / maybe_calculate_aim_angles: inherited compatibility wrappers
};
```

`AimSolution` carries `angles` (degrees, pitch positive up), `aim_point` (on the eye's aim ray at the target's distance), `predicted_target_position` and `time_of_flight`. See [the interface](projectile_engine.md).

The engine is copyable and assignable, so it can live in containers or be swapped at runtime.

### Constructor parameters

* `gravity_constant` — magnitude of gravity in world units/s² (e.g. `800.f` for Source, `9.81f` for SI). Multiplied by `projectile.m_gravity_scale` for the projectile; applied unscaled to airborne targets.
* `simulation_time_step` — Δt of the scan. Must be positive.
* `maximum_simulation_time` — search horizon in seconds. Must be positive.
* `distance_tolerance` — maximum miss distance at time `t` to accept a solution.

A non-positive step or horizon makes the engine return `std::nullopt` immediately.

---

## Trait requirements (`PredEngineConcept<EngineTrait, ArithmeticType>`)

The concept is declared in `omath/projectile_prediction/pred_engine_concept.hpp`, which this header includes. Your `EngineTrait` must expose **noexcept** static functions with these signatures (`T = ArithmeticType`):

```cpp
Vector3<T> predict_projectile_position(const Vector3<T>& launch_origin, const Projectile<T>&,
                                       T pitch_deg, T yaw_deg, T time, T gravity) noexcept;
Vector3<T> predict_target_position(const Target<T>&, T time, T gravity) noexcept;

T          calc_vector_2d_distance(const Vector3<T>& v) noexcept;      // horizontal length
T          get_vector_height_coordinate(const Vector3<T>& v) noexcept; // vertical component

ViewBasis<T> calc_view_basis(T pitch_deg, T yaw_deg) noexcept;         // forward / right / up for the angles

T          calc_direct_pitch_angle(const Vector3<T>& from, const Vector3<T>& to) noexcept; // +90 up, -90 down
T          calc_direct_yaw_angle  (const Vector3<T>& from, const Vector3<T>& to) noexcept;
```

Trait pitch is **positive upwards** for every engine, regardless of the engine's own `PitchAngle` sign. Ready-made traits exist for every supported engine, e.g. `source_engine::PredEngineTrait`, `unity_engine::PredEngineTrait`, `unreal_engine::PredEngineTrait`.

---

## Algorithm details

### Time scan

The scan runs `ceil(maximum_simulation_time / simulation_time_step)` steps with `t = simulation_time_step * i`. Computing `t` from the index rather than accumulating keeps the step count exact and avoids float drift over long horizons.

### Muzzle placement

`launch_origin = eye_origin + world_offset + forward·a + right·b + up·c` with the basis from `calc_view_basis`. Because the basis depends on the angles being solved for, the first placement uses the direct angles from the eye to the predicted target and the second uses the angles from the first solve. With offsets of a few units against targets hundreds away the second correction is far below the step tolerance, so one refinement is enough. When the muzzle offset is zero the placement and the second solve are skipped, and each step costs exactly what it did before `Launcher` existed.

### Launch pitch

With `v` = launch speed, `g = gravity_constant * m_gravity_scale`, `x` = horizontal distance and `y` = height difference from the muzzle to the predicted target:

* `g == 0` → `EngineTrait::calc_direct_pitch_angle(launch_origin, T)`.
* Discriminant `D = v⁴ - g(gx² + 2yv²) < 0` → no real solution for this step.
* `x == 0` (straight up or down) → `calc_direct_pitch_angle`, i.e. ±90°.
* Otherwise the low-arc root

  ```
  tan θ = (v² - √D) / (g x)
        = (g x² + 2 y v²) / (x (v² + √D))       // conjugate form used in code
  ```

  The two are algebraically identical. The conjugate form avoids subtracting two nearly equal numbers, which for fast projectiles (thousands of units/s) costs about 0.002° in `float`; the conjugate form stays within 1e-5°.

### Launch pitch offset

For weapons that fire above the crosshair (see [`Launcher`](launcher.md#launch-pitch-offset)) the solve above is unchanged: it finds the direction the round has to leave in. What changes is everything tied to the **view**: the muzzle is placed from `calc_view_basis(launch_pitch − offset, yaw)`, the reported `angles.pitch` is `launch_pitch − offset`, and the aim point is built on that view ray. A zero offset costs nothing per step.

A view pitch outside the engine's range is detected through `calc_view_basis`, which clamps the way the engine does: if the forward vector it returns no longer has the requested pitch (0.1° tolerance), the step is skipped and the scan continues.

### Aim point

`aim_point = eye_origin + calc_view_basis(pitch, yaw).forward * distance(eye_origin, predicted_target_position)`. Every point on that ray projects to the same pixel, so the angles and the point can never disagree, whatever the muzzle offset.

---

## Usage example

```cpp
using namespace omath::projectile_prediction;

const ProjPredEngineLegacy<> solver(/*gravity*/ 800.f, /*dt*/ 1.f / 1000.f, /*horizon*/ 10.f, /*tolerance*/ 5.f);

constexpr Projectile<float> proj{.m_launch_speed = 3000.f, .m_gravity_scale = 0.5f};
constexpr Launcher<float> launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = {.forward = 16, .right = 8, .up = -6}};
constexpr Target<float> tgt{.m_origin = {900, 120, 0}, .m_velocity = {-40, 15, 0}, .m_is_airborne = false};

if (const auto aim = solver.maybe_calculate_aim(proj, launcher, tgt))
{
    // aim->angles.pitch / yaw in degrees, aim->aim_point, aim->time_of_flight
}
```

Code written against the previous interface keeps working: `maybe_calculate_aim_point(proj, tgt)` and `maybe_calculate_aim_angles(proj, tgt)` read the eye from `proj.m_origin` and treat `proj.m_launch_offset` as a fixed world-space offset.

---

## Behavior & edge cases

* **Zero gravity** → straight-line solution via the direct pitch.
* **Straight up or down** → direct pitch (±90°); the reach check still has to pass.
* **Negative discriminant** → that step is skipped; if every step fails, `std::nullopt`.
* **Pitch offset pushes the view past the engine's limit** (e.g. below −89° in Source for a target almost straight down) → that step is skipped.
* **Tolerance** controls acceptance; tighten for accuracy, loosen for robustness. It is compared squared, so no square root runs per step.

---

## Complexity & tuning

* Time: **O(N)**, `N = ceil(maximum_simulation_time / simulation_time_step)`. Steps whose discriminant is negative cost a few flops; steps that reach the validation call the trait's projectile predictor, which dominates.
* A view-relative muzzle offset roughly doubles the per-step cost (two basis evaluations and two pitch solves).
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
* Fire from `launcher.launch_origin(basis(angles))` with the returned angles and check the miss at `time_of_flight`.
* Camera angles towards `aim_point` equal `angles`, including with a lateral muzzle offset.
* With a pitch offset, fire `forward * a + up * b` from the rotated muzzle using the returned view angles and check the miss.

---

*Last updated: 18 Sep 2026*
