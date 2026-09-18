# `omath::frostbite_engine::PredEngineTrait` — projectile prediction trait

> Header: `omath/engines/frostbite_engine/traits/pred_engine_trait.hpp`
> Namespace: `omath::frostbite_engine`
> Purpose: provide Frostbite-specific projectile and target prediction for ballistic calculations

---

## Summary

`PredEngineTrait` implements engine-specific helpers for **projectile prediction**:

* `predict_projectile_position` – computes where a projectile will be after `time` seconds
* `predict_target_position` – computes where a moving target will be after `time` seconds
* `calc_vector_2d_distance` – horizontal distance (X/Z plane, ignoring Y)
* `get_vector_height_coordinate` – extracts vertical coordinate (Y in Frostbite)
* `calc_view_basis` – forward, right and up of the view frame for a pitch and yaw
* `calc_direct_pitch_angle` – pitch angle to look from origin to target
* `calc_direct_yaw_angle` – yaw angle to look from origin to target

These methods satisfy the `PredEngineConcept` required by `ProjPredEngineLegacy`.

---

## API

```cpp
namespace omath::frostbite_engine {

class PredEngineTrait final {
public:
  // Predict projectile position after `time` seconds, launched from `launch_origin`
  static constexpr Vector3<float>
  predict_projectile_position(const Vector3<float>& launch_origin,
                             const projectile_prediction::Projectile& projectile,
                             float pitch, float yaw, float time,
                             float gravity) noexcept;

  // Predict target position after `time` seconds
  static constexpr Vector3<float>
  predict_target_position(const projectile_prediction::Target& target,
                         float time, float gravity) noexcept;

  // Compute horizontal (2D) distance
  static float
  calc_vector_2d_distance(const Vector3<float>& delta) noexcept;

  // Get vertical coordinate (Y in Frostbite)
  static constexpr float
  get_vector_height_coordinate(const Vector3<float>& vec) noexcept;

  // Forward, right and up of the view frame for the given angles
  static constexpr projectile_prediction::ViewBasis<float>
  calc_view_basis(float pitch, float yaw) noexcept;

  // Compute pitch angle to look at target
  static float
  calc_direct_pitch_angle(const Vector3<float>& origin,
                         const Vector3<float>& view_to) noexcept;

  // Compute yaw angle to look at target
  static float
  calc_direct_yaw_angle(const Vector3<float>& origin,
                       const Vector3<float>& view_to) noexcept;
};

} // namespace omath::frostbite_engine
```

---

## Projectile prediction

```cpp
auto pos = PredEngineTrait::predict_projectile_position(
  launch_origin, // where the projectile spawns (see Launcher)
  projectile,    // speed, gravity scale
  pitch_deg,     // launch pitch (positive = up)
  yaw_deg,       // launch yaw
  time,          // time in seconds
  gravity        // gravity constant (m/s²)
);
```

Computes:

1. Forward vector from pitch/yaw (using `forward_vector`; Frostbite's own pitch is negative upwards, so the trait negates it)
2. Initial velocity: `forward * launch_speed`
3. Position after `time`: `launch_origin + velocity*time - 0.5*gravity*gravityScale*time²` (Y component only)

---

## Target prediction

```cpp
auto pos = PredEngineTrait::predict_target_position(target, time, gravity);
```

Linear extrapolation plus gravity if the target is airborne:

```
predicted = origin + velocity * time
if (airborne)
  predicted.y -= 0.5 * gravity * time²
```

---

## Distance & height helpers

* `calc_vector_2d_distance(delta)` → `sqrt(delta.x² + delta.z²)` (horizontal distance)
* `get_vector_height_coordinate(vec)` → `vec.y` (vertical coordinate)

---

## Aim angle calculation

* `calc_direct_pitch_angle(origin, target)` → `asin(direction.y)` in degrees; positive = looking up
* `calc_direct_yaw_angle(origin, target)` → `atan2(direction.x, direction.z)` in degrees

---

## View basis

```cpp
auto basis = PredEngineTrait::calc_view_basis(pitch_deg, yaw_deg);
// basis.forward, basis.right, basis.up
```

Rotates the engine's world axes by the given view angles (trait pitch is positive upwards). `Launcher::launch_origin(basis)` uses it to place a view-relative muzzle offset, and the engines use `basis.forward` to build the aim point on the eye's aim ray.

---

## Conventions

* **Coordinate system**: Y-up, forward +Z, right +X
* **Trait pitch**: +90° = straight up, -90° = straight down (the engine's own `PitchAngle` is the opposite sign)
* **Gravity**: applied along -Y

---

## See also

* `omath/engines/frostbite_engine/formulas.hpp` — direction vectors and matrix builders
* [`Launcher`](../../projectile_prediction/launcher.md), [`Projectile`](../../projectile_prediction/projectile.md), [`Target`](../../projectile_prediction/target.md)
* [`ProjPredEngineLegacy`](../../projectile_prediction/proj_pred_engine_legacy.md)

---

*Last updated: 18 Sep 2026*
