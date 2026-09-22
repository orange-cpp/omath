# `omath::projectile_prediction::ProjPredEngineDrag` — Aim solver for rounds with air drag

> Header: `omath/projectile_prediction/proj_pred_engine_drag.hpp`
> Namespace: `omath::projectile_prediction`
> Inherits: `ProjPredEngineInterface<ArithmeticType>`
> Template: `ProjPredEngineDrag<EngineTrait = source_engine::PredEngineTrait, ArithmeticType = float>`
> Purpose: solve the **view angles** (and a matching aim point) for a projectile that **loses speed to the air**, by flying the round through [`ProjectileFlight`](projectile_flight.md) instead of assuming a parabola.

The closed-form engines pick their launch pitch from a parabola, and there is no closed form once drag is involved. For Team Fortress 2's pipe the parabola is 90 units long at a level view and over 500 at 40° up, because the error grows with the flight time. This engine returns the same `AimSolution` through the same interface, so it is interchangeable with [`ProjPredEngineLegacy`](proj_pred_engine_legacy.md) to everything downstream.

Use it for rounds that have drag (`Projectile::has_drag()`), and for rounds with **no gravity**: a straight flight stepped at any rate is exact, and this engine closes in on its answer instead of hoping a scan step lands inside the tolerance, so it cannot come back empty for a target that closes faster than the scan allowed for (see [below](#why-not-the-scan-for-rockets)). What it is wrong for is a round with gravity and no drag in a game that moves such rounds on a true parabola: the stepped fall drops a little further than that, and the closed-form engines are exact.

---

## Overview

Where the target will be depends on how long the round takes, and how long the round takes depends on where the target will be. The engine settles both together:

1. **Guess the flight time** as eye-to-target distance over launch speed.
2. **Predict the target** at that time with `EngineTrait::predict_target_position(target, t, g)`.
3. **Find the yaw** from where the muzzle ends up, since a muzzle that sits off to one side swings with the yaw.
4. **Find the view pitch** that flies the round through that point (see below), which also gives the round's real arrival time.
5. **Solve for the time** at which those agree: `mismatch(t) = arrival(t) − t = 0`. The first step feeds the arrival time back in; every one after it is a **secant** step through the last two mismatches, until the target stops moving between passes (closer than `distance_tolerance / 64`). Three or four passes is typical, twelve is the cap.
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
                              ArithmeticType distance_tolerance,
                              Arc arc = Arc::LOW) noexcept;

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
* `arc` — which of the two pitches that reach the target to solve for. `Arc::LOW` (the default) is the flat, quick shot. `Arc::HIGH` is the steep lob that drops onto the target from above; it takes two or three times as long to arrive and is refused, like any other shot, wherever its flight would outlast `maximum_simulation_time`, which for a fused round rules the lob out to anything near. A round without gravity flies one straight line and gets it whichever arc is asked for.

A non-positive step, horizon or launch speed returns `std::nullopt` immediately.

---

## Algorithm details

### Solving for the time

Feeding each arrival time back in as the next guess is the obvious loop, and it only works while the target is slow. It closes in by the ratio of the target's speed along the line of fire to the round's every pass: quick at walking pace, a crawl for a target blown away at two thirds of the round's speed, and **divergent** for one blown towards the shooter faster than the round flies, where each pass overshoots by more than the last. The secant step has no such ratio to depend on.

Two things keep it from giving up on a shot that exists:

* **A step can land on a time the target cannot be reached at**, although the answer is one it can. The engine backs off halfway towards the last time that could be shot at, twice at most.
* **The first guess can be such a time**: somebody running in from out of range, a jumper coming down into it. A shot that fails has failed for being too far, so the engine looks along the horizon for the time that brings the target nearest and tries that, once, and only if it is nearer than the first guess was. For a target on the ground only **later** times are looked at: the guess is how long a straight flight at launch speed takes, no round gets anywhere sooner, so a target that is only nearer earlier on is leaving and cannot be caught. For an airborne one every time is looked at, because it is falling, and the lower it gets the sooner it is reached however far off that is. A target that is simply out of reach costs one failed search, not one per time looked at.

### The flight

Every probe is a real flight through [`ProjectileFlight`](projectile_flight.md): gravity, then drag, then the move, one step at a time. The drag coefficients and the speed cap live on the [`Projectile`](projectile.md).

### View pitch search

For a fixed target point and yaw, the engine looks for the view pitch on its arc whose flight passes through the point:

1. **Start** from the pitch a drag-free round would need on that arc: the closed-form root `ProjPredEngineLegacy` solves with, less `launcher.launch_pitch_offset`. If even a parabola cannot reach, drag only shortens the reach, so there is no solution. Every pass starts here, from its own target point. Carrying the last pass's pitch over instead is a trap: a step in time can move the target by tens of degrees, and a search started on the wrong side of the arc's peak, where turning the pitch the way it expects moves the round the other way, walks itself into the limit.
2. **Fly** the round until it has covered the horizontal distance to the point, and take the height it passes at (linear within a step, as the round itself is).
3. **Walk** away from the start in whichever direction the miss says, doubling the step (0.5° first, 4° at most) until the round passes on the other side. Turning the pitch towards the peak (up on the low arc, down on the high one) has to raise the round at the target; once it stops doing that the arc is over its peak and the target is out of reach.
4. **Close in** between the pass below and the pass above with regula falsi and the Illinois correction, 24 refinements at most.

After the first pass the walk is usually skipped. Drag asks for about the same extra loft over the parabola from one pass to the next, so a shade past the last loft is tried as a **hint**: if the round passes on the other side of the target from there, that pair is the bracket. The hint is only ever paired with the pass from the parabola's pitch, which is known to be on the right arc, and is dropped if it does not bracket, so a wrong one costs a flight and can never turn a shot down.

On the high arc two more things hold. The real answer lies between the two parabola pitches (drag wants more loft than the low one and stands less than the high one), so the walk never goes below the low one. And the parabola's high pitch to a near target is all but vertical, a lob that with drag on top takes longer than any probe flight is allowed: the start is first capped at the pitch whose parabola takes the whole time limit to cover the distance, and if that is still too slow the search comes down 4° at a time until a flight gets there. A first flight that gets there and is already over the target means the crossing lies steeper still, among flights too long to count, and the shot is refused. A high-arc solve costs several times a low-arc one, 70–250 µs for a sticky or a pipe, most of it in those long probe flights.

The muzzle is placed from each probe's own angles, so the way it swings with the view is part of the answer rather than an error in it. Probe flights are allowed to run past the horizon, to `1.25 × horizon`, so that a shot right at it still has a pass on either side to close in from: the pass above is at most four degrees steeper than the answer, which gets it there a tenth or so later. Any further only makes a flight that was never going to arrive take longer to give up. The horizon itself is enforced on the answer.

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

## Why not the scan for rockets

`ProjPredEngineLegacy` accepts the first scan step that lands within `distance_tolerance`. Between two steps the gap closes by `(round speed + target's closing speed) × simulation_time_step`, and once that exceeds `2 × distance_tolerance` a step can jump clean over the window, in which case every step fails and the solve returns nothing. With a 1980 units/s rocket, a 5 ms step and a tolerance of 5, that lost **3–5 %** of solves against a walking target and **20–30 %** against one blown towards the shooter, in measurement; this engine lost none of either. Keeping `simulation_time_step ≤ 2 × distance_tolerance / (speed + fastest closing speed)` avoids it, at the cost of that many more steps; this engine has no such condition.

---

## Accuracy

The constants above were measured rather than estimated: Team Fortress 2's own `vphysics.dll` was run offline on the game's real grenade hull, launched with the calls the game makes. Fired through that engine, shots solved here pass within **0.3 units (median) and 1.9 (worst)** of the predicted target at the predicted time, across 250–1250 units, ±35° of elevation, moving and airborne targets, and the farthest level target accepted (1350 units) matches the engine's real reach (1352).

What is left is the game's own dice. It spins every pipe differently, which moves a landing by about 1%, and roughly one pipe in eight draws next to no yaw spin, flies nose-first and carries up to 12% further. That is decided on the server and cannot be aimed for.

---

## Complexity & tuning

* Each probe flight costs one step per `simulation_time_step` of flight. A solve is a handful of time passes, each a handful of probes: **8–15 µs** for a Team Fortress 2 pipe on a desktop CPU, 3–17 µs for a rocket between 400 and 3200 units.
* A shot that is refused costs more than one that is made, because it takes a failed search to be sure: typically 5–45 µs.
* Both have a long tail, and it is the same one: a target right at the edge of the round's reach, where the time search steps in and out of what can be shot at. A median pipe solve flies about 600 steps, the 99th percentile 4000–7000, and the worst refusals near 30 000 (around 170 µs). Callers that solve many targets every frame should solve once per physics tick rather than once per rendered frame, which is where most of the cost goes.
* Unlike the scan engines, cost does not grow with `maximum_simulation_time` for targets that are close.
* A looser `distance_tolerance` stops both searches earlier.

---

## Testing checklist

* Fly the returned angles with `predict_projectile_position` to `time_of_flight`: the miss is far inside `distance_tolerance`.
* The same round without drag → agrees with `ProjPredEngineLegacy` to within the extra the stepped fall drops.
* With drag → more pitch and more time than the parabola for the same target.
* Moving and airborne targets → `predicted_target_position` equals the trait's prediction at `time_of_flight`.
* Beyond the round's reach, past the horizon, or past the engine's view pitch limit → `nullopt`.
* A target closing faster than the round flies, and one fleeing at two thirds of its speed → solved, and the flight lands.
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
