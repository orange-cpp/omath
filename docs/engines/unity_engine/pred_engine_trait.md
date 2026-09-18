# `omath::unity_engine::PredEngineTrait` — projectile prediction trait

> Header: `omath/engines/unity_engine/traits/pred_engine_trait.hpp`
> Namespace: `omath::unity_engine`
> Purpose: provide Unity Engine-specific projectile and target prediction for ballistic calculations

---

## Summary

`PredEngineTrait` implements engine-specific helpers for **projectile prediction**:

* `predict_projectile_position` – computes where a projectile will be after `time` seconds
* `predict_target_position` – computes where a moving target will be after `time` seconds
* `calc_vector_2d_distance` – horizontal distance (X/Z plane, ignoring Y)
* `get_vector_height_coordinate` – extracts vertical coordinate (Y in Unity Engine)
* `calc_view_basis` – forward, right and up of the view frame for a pitch and yaw
* `calc_direct_pitch_angle` – pitch angle to look from origin to target
* `calc_direct_yaw_angle` – yaw angle to look from origin to target

These methods satisfy the `PredEngineTraitConcept` required by generic projectile prediction algorithms.

---

## API

```cpp
namespace omath::unity_engine {

class PredEngineTrait final {
public:
  // Predict projectile position after `time` seconds
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

  // Get vertical coordinate (Y in Unity Engine)
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

} // namespace omath::unity_engine
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
  gravity        // gravity constant (e.g., 9.81 m/s²)
);
```

Computes:

1. Forward vector from pitch/yaw (using `forward_vector`)
2. Initial velocity: `forward * launch_speed`
3. Position after `time`: `origin + velocity*time - 0.5*gravity*gravityScale*time²` (Y component only)

**Note**: Negative pitch in `forward_vector` convention → positive pitch looks up.

---

## Target prediction

```cpp
auto pos = PredEngineTrait::predict_target_position(
  target,   // position, velocity, airborne flag
  time,     // time in seconds
  gravity   // gravity constant
);
```

Simple linear extrapolation plus gravity if target is airborne:

```
predicted = origin + velocity * time
if (airborne)
  predicted.y -= 0.5 * gravity * time²
```

---

## Distance & height helpers

* `calc_vector_2d_distance(delta)` → `sqrt(delta.x² + delta.z²)` (horizontal distance)
* `get_vector_height_coordinate(vec)` → `vec.y` (vertical coordinate in Unity Engine)

Used to compute ballistic arc parameters.

---

## Aim angle calculation

* `calc_direct_pitch_angle(origin, target)` → pitch in degrees to look from `origin` to `target`
  - Formula: `asin(Δy / distance)` converted to degrees (direction normalized first)
  - Positive = looking up, negative = looking down

* `calc_direct_yaw_angle(origin, target)` → yaw in degrees to look from `origin` to `target`
  - Formula: `atan2(Δx, Δz)` converted to degrees (direction normalized first)
  - Horizontal rotation around Y-axis

---

## View basis

```cpp
auto basis = PredEngineTrait::calc_view_basis(pitch_deg, yaw_deg);
// basis.forward, basis.right, basis.up
```

Rotates the engine's world axes by the given view angles (trait pitch is positive upwards). `Launcher::launch_origin(basis)` uses it to place a view-relative muzzle offset, and the engines use `basis.forward` to build the aim point on the eye's aim ray.

---

## Conventions

* **Coordinate system**: Y-up (height increases with Y)
* **Angles**: pitch in [-90°, +90°], yaw in [-180°, +180°]
* **Gravity**: applied downward along -Y axis
* **Pitch convention**: +90° = straight up, -90° = straight down

---

## Usage example

```cpp
using namespace omath::unity_engine;
using namespace omath::projectile_prediction;

Projectile proj{
  .m_origin = {0, 2, 0},
  .m_launch_speed = 50.0f,
  .m_gravity_scale = 1.0f
};

Target tgt{
  .m_origin = {20, 2, 15},
  .m_velocity = {1, 0, 0.5f},
  .m_is_airborne = false
};

float gravity = 9.81f;
float time = 0.5f;

// Predict where target will be
auto target_pos = PredEngineTrait::predict_target_position(tgt, time, gravity);

// Compute aim angles
float pitch = PredEngineTrait::calc_direct_pitch_angle(proj.m_origin, target_pos);
float yaw   = PredEngineTrait::calc_direct_yaw_angle(proj.m_origin, target_pos);

// Predict projectile position with those angles
auto proj_pos = PredEngineTrait::predict_projectile_position(proj.m_origin, proj, pitch, yaw, time, gravity);
```

---

## See also

* `omath/engines/unity_engine/formulas.hpp` — direction vectors and matrix builders
* `omath/projectile_prediction/projectile.hpp` — `Projectile` struct
* `omath/projectile_prediction/target.hpp` — `Target` struct
* Generic projectile prediction algorithms that use `PredEngineTraitConcept`
