# `omath::projectile_prediction::ProjPredEngineDrag` — Aim solver for rounds with air drag

> Header: `omath/projectile_prediction/proj_pred_engine_drag.hpp`
> Namespace: `omath::projectile_prediction`
> Inherits: `ProjPredEngineInterface<ArithmeticType>`
> Template: `ProjPredEngineDrag<EngineTrait = source_engine::PredEngineTrait, ArithmeticType = float>`
> Purpose: solve the **view angles** (and a matching aim point) for a projectile that **loses speed to the air**, by flying the round through [`ProjectileFlight`](projectile_flight.md) instead of assuming a parabola.

The closed-form engines pick their launch pitch from a parabola, and there is no closed form once drag is involved. For Team Fortress 2's pipe the parabola is 90 units long at a level view and over 500 at 40° up, because the error grows with the flight time. This engine returns the same `AimSolution` through the same interface, so it is interchangeable with [`ProjPredEngineLegacy`](proj_pred_engine_legacy.md) to everything downstream.

Use it for rounds that have drag (`Projectile::has_drag()`). A round without drag is better served by the closed-form engines, which are exact for it and do not depend on the game's physics step.

---

## Overview

Where the target will be depends on how long the round takes, and how long the round takes depends on where the target will be. The engine settles both together:

1. **Guess the flight time** as eye-to-target distance over launch speed.
2. **Predict the target** at that time with `EngineTrait::predict_target_position(target, t, g)`.
3. **Find the yaw** from where the muzzle ends up, since a muzzle that sits off to one side swings with the yaw.
4. **Find the view pitch** that flies the round through that point (see below), which also gives the round's real arrival time.
5. **Repeat from 2** with the arrival time, until the target stops moving between passes (closer than `distance_tolerance / 64`). Nothing on foot outruns a projectile, so the error shrinks by about the ratio of the two speeds per pass: three or four passes is typical, twelve is the cap.
6. **Check the answer**: the horizon, the engine's view pitch limit, and that the flight really is within `distance_tolerance` of the target at the time claimed.

If any stage fails the result is `std::nullopt`.

---

## API

```cpp
template<class EngineTrait = source_engine::PredEngineTrait, class ArithmeticType = float>
requires PredEngineConcept<EngineTrait, ArithmeticType>
class ProjPredEngineDrag final : public ProjPredEngineInterface<ArithmeticType> {
public:
  explicit ProjPredEngineDrag(ArithmeticType gravity_constant,
                              ArithmeticType simulation_time_step,
                              ArithmeticType maximum_simulation_time,
                              ArithmeticType distance_tolerance) noexcept;

  [[nodiscard]] std::optional<AimSolution<ArithmeticType>>
  maybe_calculate_aim(const Projectile<ArithmeticType>&, const Launcher<ArithmeticType>&,
                      const Target<ArithmeticType>&) const noexcept override;

  // Where a round fired along view_angles is after `time` seconds
  [[nodiscard]] Vector3<ArithmeticType>
  predict_projectile_position(const Projectile<ArithmeticType>&, const Launcher<ArithmeticType>&,
                              const AimAngles<ArithmeticType>& view_angles, ArithmeticType time) const noexcept;
};
```

The engine is copyable and assignable, and works with any trait satisfying `PredEngineConcept` (declared in `omath/projectile_prediction/pred_engine_concept.hpp`), in `float` or `double`.

### Constructor parameters

* `gravity_constant` — magnitude of gravity in world units/s². Multiplied by `projectile.m_gravity_scale` for the round; applied unscaled to airborne targets.
* `simulation_time_step` — **the physics step of the game being predicted**, not an accuracy setting. Drag is integrated the way the game integrates it, so the arc only matches when it is stepped at the game's own rate: Source steps VPhysics once per tick, `0.015` s for Team Fortress 2. This differs from `ProjPredEngineLegacy`, where the same parameter is the resolution of a scan.
* `maximum_simulation_time` — longest flight accepted, in seconds. For a fused round this is the fuse.
* `distance_tolerance` — maximum miss accepted at the time of flight. It also sets how hard the solver works: the pitch search stops within `distance_tolerance / 100` of the target's height, and the time loop within `distance_tolerance / 64` of target drift.

A non-positive step, horizon or launch speed returns `std::nullopt` immediately.

---

## Algorithm details

### The flight

Every probe is a real flight through [`ProjectileFlight`](projectile_flight.md): gravity, then drag, then the move, one step at a time. The drag coefficients and the speed cap live on the [`Projectile`](projectile.md).

### View pitch search

For a fixed target point and yaw, the engine looks for the **low-arc** view pitch whose flight passes through the point:

1. **Start** from the pitch a drag-free round would need: the same closed-form low-arc root `ProjPredEngineLegacy` solves with, less `launcher.launch_pitch_offset`. If even a parabola cannot reach, drag only shortens the reach, so there is no solution.
2. **Fly** the round until it has covered the horizontal distance to the point, and take the height it passes at (linear within a step, as the round itself is).
3. **Walk** away from the start in whichever direction the miss says, doubling the step (0.5° first, 4° at most) until the round passes on the other side. Raising the pitch has to raise the round at the target; once it stops doing that the arc is over its peak and the target is out of reach.
4. **Close in** between the pass below and the pass above with regula falsi and the Illinois correction, 24 refinements at most.

The muzzle is placed from each probe's own angles, so the way it swings with the view is part of the answer rather than an error in it. Probe flights are allowed to run past the horizon (`1.5 × horizon + 0.5 s`) so that a shot right at it still has a pass on either side to close in from; the horizon itself is enforced on the answer.

### Engine independence

Nothing in the solver names an axis. World up is `calc_view_basis(0, 0).up`, the horizontal heading for a yaw is `calc_view_basis(0, yaw).forward`, and heights come from `get_vector_height_coordinate`, so it works unchanged for y-up engines such as Unity.

### View pitch limit

`calc_view_basis` clamps the way the engine does. If the forward vector it returns no longer has the solved pitch (0.1° tolerance) nobody can set the shot up, and the result is `std::nullopt`. This is the check `ProjPredEngineLegacy` makes.

### Aim point

`aim_point = eye_origin + calc_view_basis(pitch, yaw).forward * distance(eye_origin, predicted_target_position)`, as in the other engines.

---

## Usage example

```cpp
using namespace omath::projectile_prediction;

// Team Fortress 2 grenade launcher: forward * 1200 + up * 200, a tumbling round, VPhysics' 2000 units/s cap
constexpr Projectile<float> pipe{.m_launch_speed = 1216.5525f,
                                 .m_gravity_scale = 1.f,
                                 .m_drag = 2.838e-4f,
                                 .m_max_speed = 2000.f};

const Launcher<float> launcher{.eye_origin = eye_position,
                               .muzzle_offset = {.forward = 16.f, .right = 8.f, .up = -6.f},
                               .launch_pitch_offset = 9.46232f};

// Gravity 800, one step per 0.015 s tick, a 2.13 s fuse, 5 units of tolerance
const ProjPredEngineDrag<> engine(800.f, 0.015f, 2.13f, 5.f);

if (const auto aim = engine.maybe_calculate_aim(pipe, launcher, target))
{
    // aim->angles are view angles, pitch positive upwards
}
```

Picking the engine per round keeps the closed-form path for everything that can use it:

```cpp
if (projectile.has_drag())
    return drag_engine.maybe_calculate_aim(projectile, launcher, target);
return legacy_engine.maybe_calculate_aim(projectile, launcher, target);
```

---

## Accuracy

The constants above were measured rather than estimated: Team Fortress 2's own `vphysics.dll` was run offline on the game's real grenade hull, launched with the calls the game makes. Fired through that engine, shots solved here pass within **0.3 units (median) and 1.9 (worst)** of the predicted target at the predicted time, across 250–1250 units, ±35° of elevation, moving and airborne targets, and the farthest level target accepted (1350 units) matches the engine's real reach (1352).

What is left is the game's own dice. It spins every pipe differently, which moves a landing by about 1%, and roughly one pipe in eight draws next to no yaw spin, flies nose-first and carries up to 12% further. That is decided on the server and cannot be aimed for.

---

## Complexity & tuning

* Each probe flight costs one step per `simulation_time_step` of flight. A solve is a handful of time passes, each a handful of probes: **10–20 µs** for a Team Fortress 2 pipe on a desktop CPU.
* Unlike the scan engines, cost does not grow with `maximum_simulation_time` for targets that are close.
* A looser `distance_tolerance` stops both searches earlier.

---

## Testing checklist

* Fly the returned angles with `predict_projectile_position` to `time_of_flight`: the miss is far inside `distance_tolerance`.
* The same round without drag → agrees with `ProjPredEngineLegacy` to within the extra the stepped fall drops.
* With drag → more pitch and more time than the parabola for the same target.
* Moving and airborne targets → `predicted_target_position` equals the trait's prediction at `time_of_flight`.
* Beyond the round's reach, past the horizon, or past the engine's view pitch limit → `nullopt`.
* Non-positive step, horizon or launch speed → `nullopt`, no hang.
* A y-up trait and a `double` trait solve the same way.

---

## See also

* [`ProjectileFlight`](projectile_flight.md) — the stepper this engine flies its probes with
* [`Projectile`](projectile.md) — drag coefficients and speed cap
* [`Launcher`](launcher.md) — muzzle offset and launch pitch offset
* [`ProjPredEngineInterface`](projectile_engine.md) — common interface for aim solvers
* [`ProjPredEngineLegacy`](proj_pred_engine_legacy.md) — closed-form solver for rounds without drag

---

*Last updated: 19 Sep 2026*
