# `omath::projectile_prediction::ProjectileFlight` — One round in flight, a physics step at a time

> Header: `omath/projectile_prediction/projectile_flight.hpp`
> Namespace: `omath::projectile_prediction`
> Template: `ProjectileFlight<EngineTrait = source_engine::PredEngineTrait, ArithmeticType = float>`
> Used by: [`ProjPredEngineDrag`](proj_pred_engine_drag.md), and directly by anything that draws or traces an arc

Drag has no closed form, so a round that has it can only be followed the way the game's physics moves it. `ProjectileFlight` does that: construct it from the round, the launcher and the shooter's view angles, call `step()` once per physics step, and read `position()`.

A round without drag is a parabola, which `EngineTrait::predict_projectile_position()` reads off directly at any time.

---

## API

```cpp
template<class EngineTrait = source_engine::PredEngineTrait, class ArithmeticType = float>
requires PredEngineConcept<EngineTrait, ArithmeticType>
class ProjectileFlight final {
public:
  ProjectileFlight(const Projectile<ArithmeticType>& projectile, const Launcher<ArithmeticType>& launcher,
                   const AimAngles<ArithmeticType>& view_angles, ArithmeticType gravity_constant,
                   ArithmeticType time_step) noexcept;

  void step() noexcept;

  [[nodiscard]] const Vector3<ArithmeticType>& origin() const noexcept;   // the muzzle, not the eye
  [[nodiscard]] const Vector3<ArithmeticType>& position() const noexcept;
  [[nodiscard]] const Vector3<ArithmeticType>& velocity() const noexcept;
  [[nodiscard]] ArithmeticType time() const noexcept;                     // time_step * steps taken
};
```

### Constructor parameters

* `projectile` — launch speed, gravity scale, drag coefficients and speed cap. See [`Projectile`](projectile.md).
* `launcher` — the muzzle is placed from `view_angles` exactly as the engines place it, and the round leaves `launcher.launch_pitch_offset` above them.
* `view_angles` — the **shooter's** angles in degrees, pitch positive upwards, i.e. what `AimSolution::angles` holds. Not the launch direction.
* `gravity_constant` — the world's gravity; scaled here by `projectile.m_gravity_scale`.
* `time_step` — **the physics step of the game being predicted**, not an accuracy setting. Source steps VPhysics once per tick (`0.015` s in Team Fortress 2). A finer step is not more accurate: it is a different integration from the one the game runs.

---

## The step

```
velocity -= world_up * gravity * dt
velocity *= 1 - min(1, dt * (m_drag * |v| + m_drag_forward * |v · F| + m_drag_up * |v · U|))
velocity  = capped to m_max_speed, when that is set
position += velocity * dt
```

in that order and with no sub-steps, which is what Source's VPhysics does to a grenade. Three things follow from it:

* **Gravity lands on the velocity before the move**, so the round falls `g·dt²·n(n+1)/2` over `n` steps, slightly more than the `g·t²/2` of a true parabola (6 units after one second at `g = 800`, `dt = 0.015`).
* **Drag is proportional to speed**, so it takes a fraction of the velocity each step and that fraction shrinks as the round slows. It can stop a round but never turn it around.
* **The cap applies on launch too**: a round asked to leave faster than `m_max_speed` leaves at `m_max_speed`.

`F` and `U` are the round's own forward and up. It is spawned facing along the view and nothing in flight turns it, so they are the view's axes at launch and stay there. `world_up` is `EngineTrait::calc_view_basis(0, 0).up`, so gravity pulls along whichever axis the engine keeps height on.

### Which drag to use

* **`m_drag`** — a round that **tumbles** shows the air every side in turn, which averages out to one number. One constant follows Team Fortress 2's pipe to within 1.5 units over a 2.13 s flight at any pitch.
* **`m_drag_forward`, `m_drag_up`** — a round that **holds its attitude** (the Loch-n-Load's is fired without spin) feels end-on drag along its nose and side-on drag across it, and the arc bending away from the nose moves it from one to the other. No single constant describes that; the two-axis form is exact, to 0.04 units over two seconds.

There is no third coefficient because a round fired without roll never moves along its own right.

For Source, the engine's numbers can be read straight out of it: `IPhysicsObject::CalculateLinearDrag()` on each of the hull's axes, times `0.0254` (metres per unit) times half the air density (`2` by default). For the grenade hull that gives `0.003902` and `0.009962`, hence `9.911e-5` and `2.530e-4`.

---

## Usage example

Sampling an arc for drawing, a corner every sixth tick:

```cpp
using namespace omath::projectile_prediction;

ProjectileFlight<> flight{pipe, launcher, view_angles, 800.f, 0.015f};

std::vector<omath::Vector3<float>> arc{flight.position()};
for (int tick = 1; tick <= 142; ++tick)
{
    flight.step();
    if (tick % 6 == 0)
        arc.push_back(flight.position());
}
```

Every tick is stepped, so the arc is the one the game flies however few corners are kept. Between two steps the round is on the straight line joining them, which is how the game moves and draws it too.

For a single position at an arbitrary time, `ProjPredEngineDrag::predict_projectile_position()` does the stepping and the interpolation.

---

## Notes & edge cases

* `view_angles.pitch` goes through `EngineTrait::calc_view_basis`, which clamps the way the engine does (±89° in Source).
* With no drag, no gravity and no cap, `step()` moves the round in a straight line at its launch speed.
* A `time_step` of zero leaves the round where it is; `time()` stays at zero.

---

## See also

* [`ProjPredEngineDrag`](proj_pred_engine_drag.md) — aim solver built on this
* [`Projectile`](projectile.md) — where the drag coefficients and the speed cap live
* [`Launcher`](launcher.md) — muzzle offset and launch pitch offset

---

*Last updated: 19 Sep 2026*
