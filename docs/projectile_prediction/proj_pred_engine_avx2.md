# `omath::projectile_prediction::ProjPredEngineAvx2` — AVX2-accelerated ballistic aim solver

> Header: `omath/projectile_prediction/proj_pred_engine_avx2.hpp`
> Namespace: `omath::projectile_prediction`
> Inherits: `ProjPredEngineInterface<float>`
> Conventions: Source-style, Z up, yaw about Z (it does not take an engine trait)
> Availability: x86 / x86-64 builds with `OMATH_USE_AVX2`; every call throws `std::runtime_error` otherwise

This engine scans candidate times of flight **eight at a time** with AVX2 FMA, keeps the ones at which the target is reachable at the launch speed, and solves the elevation for the first of them. It implements the same `maybe_calculate_aim` contract as the legacy engine and returns the same `AimSolution`.

---

## API

```cpp
class ProjPredEngineAvx2 final : public ProjPredEngineInterface<float> {
public:
  ProjPredEngineAvx2(float gravity_constant, float simulation_time_step, float maximum_simulation_time) noexcept;

  [[nodiscard]]
  std::optional<AimSolution<float>>
  maybe_calculate_aim(const Projectile<float>& projectile, const Launcher<float>& launcher,
                      const Target<float>& target) const override;

  // maybe_calculate_aim_point / maybe_calculate_aim_angles: inherited compatibility wrappers
};
```

### Parameters (constructor)

* `gravity_constant` — magnitude of gravity in world units/s² (e.g. `800.f` for Source).
* `simulation_time_step` — Δt between scanned candidate times.
* `maximum_simulation_time` — cap on time of flight.

There is no distance tolerance: a time step is accepted when the speed needed to be at the predicted target at exactly that time is at most the launch speed.

---

## How it solves

1. **Place the muzzle.** `launch_origin = launcher.eye_origin + world_offset`, plus the view-relative muzzle offset rotated by the direct angles from the eye to the target's current position.
2. **Vectorised scan.** For eight times `t` at once: predict the target (`origin + velocity t`, minus `½ g t²` if airborne), take the horizontal distance `d` and the height `h` from the launch origin, and compute the speed that would be needed to be there at `t`:

   ```
   term  = h + ½ g_bullet t²
   v_req² = (d² + term²) / t²
   ```

   Lanes with `v_req² ≤ v0²` are candidates.
3. **Scalar solve** on the first candidate, searching two steps either side of it, with `pitch = atan(term / d)`.
4. **Refine once** if the muzzle offset is view-relative: move the muzzle to `launcher.launch_origin(basis(pitch, yaw))`, re-solve the pitch at the same time, recompute the yaw.
5. **Return** `angles`, `aim_point = eye + forward(angles) * distance(eye, target)`, the predicted target position and the time.

The accepted time is the first feasible step, so the projectile can be up to one step of travel away from the target at `time_of_flight`. Use a smaller step for fast projectiles.

---

## Usage example

```cpp
using namespace omath::projectile_prediction;

const ProjPredEngineAvx2 solver(/*gravity*/ 800.f, /*dt*/ 1.f / 1000.f, /*Tmax*/ 5.f);

constexpr Projectile<float> proj{.m_launch_speed = 1100.f, .m_gravity_scale = 1.f};
constexpr Launcher<float> launcher{.eye_origin = {0, 0, 64}, .muzzle_offset = {.forward = 16, .right = 8, .up = -6}};
Target<float> tgt = /* position, velocity, airborne */;

if (const auto aim = solver.maybe_calculate_aim(proj, launcher, tgt))
{
    // aim->angles.pitch / yaw, aim->aim_point
}
```

---

## Edge cases & failure modes

* **Zero or tiny launch speed** → no candidate lane, `nullopt`.
* **Target receding faster than the projectile** → `nullopt`.
* **Solutions only beyond `maximum_simulation_time`** → `nullopt`.
* **Straight above or below** (`d == 0`) → ±90°.
* **Built without AVX2** or on a non-x86 target → `std::runtime_error` on every call. Use `ProjPredEngineLegacy` there.

---

## Performance & tuning

* Work is `O(Tmax / Δt / 8)` for the scan plus a handful of scalar solves.
* Smaller `Δt` → tighter hits at linear cost.

---

## Testing checklist

* Stationary target at the same height → pitch ≈ 0.
* Higher target → positive pitch; lower target → negative pitch.
* Fire from `launcher.launch_origin(basis(angles))` with the returned angles and check the miss at `time_of_flight`.
* Camera angles towards `aim_point` equal `angles`.
* Very fast receding target → `nullopt`.

---

## See also

* [`ProjPredEngineInterface`](projectile_engine.md) — base interface and general contract
* [`ProjPredEngineLegacy`](proj_pred_engine_legacy.md) — portable trait-based engine
* [`Launcher`](launcher.md), [`Projectile`](projectile.md), [`Target`](target.md) — solver inputs

---

*Last updated: 18 Sep 2026*
