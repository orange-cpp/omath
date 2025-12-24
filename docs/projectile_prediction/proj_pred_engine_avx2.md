# `omath::projectile_prediction::ProjPredEngineAvx2` — AVX2-accelerated ballistic aim solver

> Header: your project’s `projectile_prediction/proj_pred_engine_avx2.hpp`
> Namespace: `omath::projectile_prediction`
> Inherits: `ProjPredEngineInterface`
> Depends on: `Vector3<float>`, `Projectile`, `Target`
> CPU: Uses AVX2 when available; falls back to scalar elsewhere (fields are marked `[[maybe_unused]]` for non-x86/AVX2 builds).

This engine computes a **world-space aim point** (and implicitly the firing **yaw/pitch**) to intersect a moving target under a **constant gravity** model and **constant muzzle speed**. It typically scans candidate times of flight and solves for the elevation (`pitch`) that makes the vertical and horizontal kinematics meet at the same time.

---

## API

```cpp
class ProjPredEngineAvx2 final : public ProjPredEngineInterface {
public:
  [[nodiscard]]
  std::optional<Vector3<float>>
  maybe_calculate_aim_point(const Projectile& projectile,
                            const Target& target) const override;

  ProjPredEngineAvx2(float gravity_constant,
                     float simulation_time_step,
                     float maximum_simulation_time);
  ~ProjPredEngineAvx2() override = default;

private:
  // Solve for pitch at a fixed time-of-flight t.
  [[nodiscard]]
  static std::optional<float>
  calculate_pitch(const Vector3<float>& proj_origin,
                  const Vector3<float>& target_pos,
                  float bullet_gravity, float v0, float time);

  // Tunables (may be unused on non-AVX2 builds)
  [[maybe_unused]] const float m_gravity_constant;      // |g| (e.g., 9.81)
  [[maybe_unused]] const float m_simulation_time_step;  // Δt (e.g., 1/240 s)
  [[maybe_unused]] const float m_maximum_simulation_time; // Tmax (e.g., 3 s)
};
```

### Parameters (constructor)

* `gravity_constant` — magnitude of gravity (units consistent with your world, e.g., **m/s²**).
* `simulation_time_step` — Δt used to scan candidate intercept times.
* `maximum_simulation_time` — cap on time of flight; larger allows longer-range solutions but increases cost.

### Return (solver)

* `maybe_calculate_aim_point(...)`

    * **`Vector3<float>`**: a world-space **aim point** yielding an intercept under the model.
    * **`std::nullopt`**: no feasible solution (e.g., target receding too fast, out of range, or kinematics inconsistent).

---

## How it solves (expected flow)

1. **Predict target at time `t`** (constant-velocity model unless your `Target` carries more):

   ```
   T(t) = target.position + target.velocity * t
   ```
2. **Horizontal/vertical kinematics at fixed `t`** with muzzle speed `v0` and gravity `g`:

    * Let `Δ = T(t) - proj_origin`, `d = length(Δ.xz)`, `h = Δ.y`.
    * Required initial components:

      ```
      cosθ = d / (v0 * t)
      sinθ = (h + 0.5 * g * t^2) / (v0 * t)
      ```
    * If `cosθ` ∈ [−1,1] and `sinθ` ∈ [−1,1] and `sin²θ + cos²θ ≈ 1`, then

      ```
      θ = atan2(sinθ, cosθ)
      ```

      That is what `calculate_pitch(...)` returns on success.
3. **Yaw** is the azimuth toward `Δ.xz`.
4. **Pick the earliest feasible `t`** in `[Δt, Tmax]` (scanned in steps of `Δt`; AVX2 batches several `t` at once).
5. **Return the aim point.** Common choices:

    * The **impact point** `T(t*)` (useful as a HUD marker), or
    * A point along the **initial firing ray** at some convenient range using `(yaw, pitch)`; both are consistent—pick the convention your caller expects.

> The private `calculate_pitch(...)` matches step **2** and returns `nullopt` if the trigonometric constraints are violated for that `t`.

---

## AVX2 notes

* On x86/x64 with AVX2, candidate times `t` can be evaluated **8 at a time** using FMA (great for dense scans).
* On ARM/ARM64 (no AVX2), code falls back to scalar math; the `[[maybe_unused]]` members acknowledge compilation without SIMD.

---

## Usage example

```cpp
using namespace omath::projectile_prediction;

ProjPredEngineAvx2 solver(
  /*gravity*/ 9.81f,
  /*dt*/       1.0f/240.0f,
  /*Tmax*/     3.0f
);

Projectile proj; // fill: origin, muzzle_speed, etc.
Target     tgt;  // fill: position, velocity

if (auto aim = solver.maybe_calculate_aim_point(proj, tgt)) {
  // Aim your weapon at *aim and fire with muzzle speed proj.v0
  // If you need yaw/pitch explicitly, replicate the pitch solve and azimuth.
} else {
  // No solution (out of envelope) — pick a fallback
}
```

---

## Edge cases & failure modes

* **Zero or tiny `v0`** → no solution.
* **Target collinear & receding faster than `v0`** → no solution.
* **`t` constraints**: if viable solutions exist only beyond `Tmax`, you’ll get `nullopt`.
* **Geometric infeasibility** at a given `t` (e.g., `d > v0*t`) causes `calculate_pitch` to fail that sample.
* **Numerical tolerance**: check `sin²θ + cos²θ` against 1 with a small epsilon (e.g., `1e-3`).

---

## Performance & tuning

* Work is roughly `O(Nt)` where `Nt ≈ Tmax / Δt`.
* Smaller `Δt` → better accuracy, higher cost. With AVX2 you can afford smaller steps.
* If you frequently miss solutions **between** steps, consider:

    * **Coarse-to-fine**: coarse scan, then local refine around the best `t`.
    * **Newton on time**: root-find `‖horizontal‖ − v0 t cosθ(t) = 0` shaped from the kinematics.

---

## Testing checklist

* **Stationary target** at same height → θ ≈ 0, aim point ≈ target.
* **Higher target** → positive pitch; **lower target** → negative pitch.
* **Perpendicular moving target** → feasible at moderate speeds.
* **Very fast receding target** → `nullopt`.
* **Boundary**: `d ≈ v0*Tmax` and `h` large → verify pass/fail around thresholds.

---

## See also

* `ProjPredEngineInterface` — base interface and general contract
* `Projectile`, `Target` — data carriers for solver inputs (speed, origin, position, velocity, etc.)

---

*Last updated: 24 Dec 2025*
