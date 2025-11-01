# `omath::unreal_engine::PredEngineTrait` — projectile prediction trait

> Header: `omath/engines/unreal_engine/traits/pred_engine_trait.hpp`
> Namespace: `omath::unreal_engine`
> Purpose: provide Unreal Engine-specific projectile and target prediction for ballistic calculations

---

## Summary

`PredEngineTrait` implements engine-specific helpers for **projectile prediction**:

* `predict_projectile_position` – computes where a projectile will be after `time` seconds
* `predict_target_position` – computes where a moving target will be after `time` seconds
* `calc_vector_2d_distance` – horizontal distance (X/Y plane, ignoring Z)
* `get_vector_height_coordinate` – extracts vertical coordinate (Y in Unreal Engine, note: code uses Z)
* `calc_viewpoint_from_angles` – computes aim point given pitch angle
* `calc_direct_pitch_angle` – pitch angle to look from origin to target
* `calc_direct_yaw_angle` – yaw angle to look from origin to target

These methods satisfy the `PredEngineTraitConcept` required by generic projectile prediction algorithms.

---

## API

```cpp
namespace omath::unreal_engine {

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

  // Get vertical coordinate (implementation returns Y, but UE is Z-up)
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

} // namespace omath::unreal_engine
```

---

## Projectile prediction

```cpp
auto pos = PredEngineTrait::predict_projectile_position(
  projectile,    // initial position, speed, gravity scale
  pitch_deg,     // launch pitch (positive = up)
  yaw_deg,       // launch yaw
  time,          // time in seconds
  gravity        // gravity constant (e.g., 980 cm/s²)
);
```

Computes:

1. Forward vector from pitch/yaw (using `forward_vector`)
2. Initial velocity: `forward * launch_speed`
3. Position after `time`: `origin + velocity*time - 0.5*gravity*gravityScale*time²` (Y component per implementation, though UE is Z-up)

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
  predicted.y -= 0.5 * gravity * time²  // Note: implementation uses Y
```

---

## Distance & height helpers

* `calc_vector_2d_distance(delta)` → `sqrt(delta.x² + delta.z²)` (horizontal distance in X/Z plane)
* `get_vector_height_coordinate(vec)` → `vec.y` (implementation returns Y; UE convention is Z-up)

Used to compute ballistic arc parameters.

---

## Aim angle calculation

* `calc_direct_pitch_angle(origin, target)` → pitch in degrees to look from `origin` to `target`
  - Formula: `asin(Δz / distance)` converted to degrees (direction normalized first)
  - Positive = looking up, negative = looking down

* `calc_direct_yaw_angle(origin, target)` → yaw in degrees to look from `origin` to `target`
  - Formula: `atan2(Δy, Δx)` converted to degrees (direction normalized first)
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
* **Angles**: pitch in [-90°, +90°], yaw in [-180°, +180°]
* **Gravity**: applied downward (implementation uses Y component, but UE is Z-up)
* **Pitch convention**: +90° = straight up, -90° = straight down

**Note**: Some implementation details (gravity application to Y coordinate) may need adjustment for full Unreal Engine Z-up consistency.

---

## Usage example

```cpp
using namespace omath::unreal_engine;
using namespace omath::projectile_prediction;

Projectile proj{
  .m_origin = {0, 0, 200},
  .m_launch_speed = 5000.0f,
  .m_gravity_scale = 1.0f
};

Target tgt{
  .m_origin = {2000, 1000, 200},
  .m_velocity = {50, 20, 0},
  .m_is_airborne = false
};

float gravity = 980.0f;  // cm/s² in Unreal units
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

* `omath/engines/unreal_engine/formulas.hpp` — direction vectors and matrix builders
* `omath/projectile_prediction/projectile.hpp` — `Projectile` struct
* `omath/projectile_prediction/target.hpp` — `Target` struct
* Generic projectile prediction algorithms that use `PredEngineTraitConcept`
