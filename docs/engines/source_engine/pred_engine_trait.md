# `omath::source_engine::PredEngineTrait` — projectile prediction trait

> Header: `omath/engines/source_engine/traits/pred_engine_trait.hpp`
> Namespace: `omath::source_engine`
> Purpose: provide Source Engine-specific projectile and target prediction for ballistic calculations

---

## Summary

`PredEngineTrait` implements engine-specific helpers for **projectile prediction**:

* `predict_projectile_position` – computes where a projectile will be after `time` seconds
* `predict_target_position` – computes where a moving target will be after `time` seconds
* `calc_vector_2d_distance` – horizontal distance (X/Y plane, ignoring Z)
* `get_vector_height_coordinate` – extracts vertical coordinate (Z in Source Engine)
* `calc_viewpoint_from_angles` – computes aim point given pitch angle
* `calc_direct_pitch_angle` – pitch angle to look from origin to target
* `calc_direct_yaw_angle` – yaw angle to look from origin to target

These methods satisfy the `PredEngineTraitConcept` required by generic projectile prediction algorithms.

---

## API

```cpp
namespace omath::source_engine {

class PredEngineTrait final {
public:
  // Predict projectile position after `time` seconds
  static constexpr Vector3<float>
  predict_projectile_position(const projectile_prediction::Projectile& projectile,
                             float pitch, float yaw, float time,
                             float gravity) noexcept;

  // Predict target position after `time` seconds
  static constexpr Vector3<float>
  predict_target_position(const projectile_prediction::Target& target,
                         float time, float gravity) noexcept;

  // Compute horizontal (2D) distance
  static float
  calc_vector_2d_distance(const Vector3<float>& delta) noexcept;

  // Get vertical coordinate (Z in Source Engine)
  static constexpr float
  get_vector_height_coordinate(const Vector3<float>& vec) noexcept;

  // Compute aim point from angles
  static Vector3<float>
  calc_viewpoint_from_angles(const projectile_prediction::Projectile& projectile,
                            Vector3<float> predicted_target_position,
                            std::optional<float> projectile_pitch) noexcept;

  // Compute pitch angle to look at target
  static float
  calc_direct_pitch_angle(const Vector3<float>& origin,
                         const Vector3<float>& view_to) noexcept;

  // Compute yaw angle to look at target
  static float
  calc_direct_yaw_angle(const Vector3<float>& origin,
                       const Vector3<float>& view_to) noexcept;
};

} // namespace omath::source_engine
```

---

## Projectile prediction

```cpp
auto pos = PredEngineTrait::predict_projectile_position(
  projectile,    // initial position, speed, gravity scale
  pitch_deg,     // launch pitch (positive = up)
  yaw_deg,       // launch yaw
  time,          // time in seconds
  gravity        // gravity constant (e.g., 800 units/s²)
);
```

Computes:

1. Forward vector from pitch/yaw (using `forward_vector`)
2. Initial velocity: `forward * launch_speed`
3. Position after `time`: `origin + velocity*time - 0.5*gravity*gravityScale*time²` (Z component only)

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
  predicted.z -= 0.5 * gravity * time²
```

---

## Distance & height helpers

* `calc_vector_2d_distance(delta)` → `sqrt(delta.x² + delta.y²)` (horizontal distance)
* `get_vector_height_coordinate(vec)` → `vec.z` (vertical coordinate in Source Engine)

Used to compute ballistic arc parameters.

---

## Aim angle calculation

* `calc_direct_pitch_angle(origin, target)` → pitch in degrees to look from `origin` to `target`
  - Formula: `asin(Δz / distance)` converted to degrees
  - Positive = looking up, negative = looking down

* `calc_direct_yaw_angle(origin, target)` → yaw in degrees to look from `origin` to `target`
  - Formula: `atan2(Δy, Δx)` converted to degrees
  - Horizontal rotation around Z-axis

---

## Viewpoint from angles

```cpp
auto aim_point = PredEngineTrait::calc_viewpoint_from_angles(
  projectile,
  predicted_target_pos,
  optional_pitch_deg
);
```

Computes where to aim in 3D space given a desired pitch angle. Uses horizontal distance and `tan(pitch)` to compute height offset.

---

## Conventions

* **Coordinate system**: Z-up (height increases with Z)
* **Angles**: pitch in [-89°, +89°], yaw in [-180°, +180°]
* **Gravity**: applied downward along -Z axis
* **Pitch convention**: +89° = straight up, -89° = straight down

---

## Usage example

```cpp
using namespace omath::source_engine;
using namespace omath::projectile_prediction;

Projectile proj{
  .m_origin = {0, 0, 100},
  .m_launch_speed = 1000.0f,
  .m_gravity_scale = 1.0f
};

Target tgt{
  .m_origin = {500, 200, 100},
  .m_velocity = {10, 5, 0},
  .m_is_airborne = false
};

float gravity = 800.0f;
float time = 0.5f;

// Predict where target will be
auto target_pos = PredEngineTrait::predict_target_position(tgt, time, gravity);

// Compute aim angles
float pitch = PredEngineTrait::calc_direct_pitch_angle(proj.m_origin, target_pos);
float yaw   = PredEngineTrait::calc_direct_yaw_angle(proj.m_origin, target_pos);

// Predict projectile position with those angles
auto proj_pos = PredEngineTrait::predict_projectile_position(proj, pitch, yaw, time, gravity);
```

---

## See also

* `omath/engines/source_engine/formulas.hpp` — direction vectors and matrix builders
* `omath/projectile_prediction/projectile.hpp` — `Projectile` struct
* `omath/projectile_prediction/target.hpp` — `Target` struct
* Generic projectile prediction algorithms that use `PredEngineTraitConcept`
