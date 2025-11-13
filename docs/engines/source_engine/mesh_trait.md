# `omath::source_engine::MeshTrait` — mesh transformation trait for Source Engine

> Header: `omath/engines/source_engine/traits/mesh_trait.hpp`
> Namespace: `omath::source_engine`
> Purpose: provide Source Engine-specific rotation matrix computation for `omath::primitives::Mesh`

---

## Summary

`MeshTrait` is a trait class that provides the `rotation_matrix` function for transforming meshes in Source Engine's coordinate system. It serves as a template parameter to `omath::primitives::Mesh`, enabling engine-specific rotation behavior.

---

## Coordinate System

**Source Engine** uses:
* **Up axis**: +Z
* **Forward axis**: +Y
* **Right axis**: +X
* **Handedness**: Right-handed
* **Rotation order**: Pitch (X) → Yaw (Z) → Roll (Y)

---

## API

```cpp
namespace omath::source_engine {

class MeshTrait final {
public:
    [[nodiscard]]
    static Mat4X4 rotation_matrix(const ViewAngles& rotation);
};

} // namespace omath::source_engine
```

---

## Method: `rotation_matrix`

```cpp
static Mat4X4 rotation_matrix(const ViewAngles& rotation);
```

Computes a 4×4 rotation matrix from Source Engine-style Euler angles.

**Parameters**:
* `rotation` — `ViewAngles` containing pitch, yaw, and roll angles

**Returns**: 4×4 rotation matrix suitable for mesh transformation

**Implementation**: Delegates to `source_engine::rotation_matrix(rotation)` defined in `formulas.hpp`.

---

## Usage

### With Mesh

```cpp
using namespace omath::source_engine;

// Create mesh (MeshTrait is used automatically)
Mesh my_mesh(vertices, indices);

// Set rotation using ViewAngles
ViewAngles angles;
angles.pitch = PitchAngle::from_degrees(30.0f);
angles.yaw = YawAngle::from_degrees(45.0f);
angles.roll = RollAngle::from_degrees(0.0f);

my_mesh.set_rotation(angles);

// The rotation matrix is computed using MeshTrait::rotation_matrix
auto matrix = my_mesh.get_to_world_matrix();
```

### Direct Usage

```cpp
using namespace omath::source_engine;

ViewAngles angles;
angles.pitch = PitchAngle::from_degrees(45.0f);
angles.yaw = YawAngle::from_degrees(90.0f);
angles.roll = RollAngle::from_degrees(0.0f);

Mat4X4 rot_matrix = MeshTrait::rotation_matrix(angles);

// Use the matrix directly
Vector3<float> local_point{1, 0, 0};
auto rotated = rot_matrix * mat_column_from_vector(local_point);
```

---

## Rotation Conventions

The rotation matrix is built following Source Engine's conventions:

1. **Pitch** (rotation around X-axis / right axis)
   * Positive pitch looks upward (+Z direction)
   * Range: typically [-89°, 89°]

2. **Yaw** (rotation around Z-axis / up axis)
   * Positive yaw rotates counterclockwise when viewed from above
   * Range: [-180°, 180°]

3. **Roll** (rotation around Y-axis / forward axis)
   * Positive roll tilts right
   * Range: [-180°, 180°]

**Composition**: The matrices are combined in the order Pitch × Yaw × Roll, producing a rotation that:
* First applies roll around the forward axis
* Then applies yaw around the up axis
* Finally applies pitch around the right axis

This matches Source Engine's internal rotation order.

---

## Related Functions

The trait delegates to the formula defined in `formulas.hpp`:

```cpp
[[nodiscard]]
Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept;
```

See [Formulas Documentation](formulas.md) for details on the rotation matrix computation.

---

## Type Alias

The Source Engine mesh type is pre-defined:

```cpp
namespace omath::source_engine {
    using Mesh = primitives::Mesh<Mat4X4, ViewAngles, MeshTrait, float>;
}
```

Use this alias to ensure correct trait usage:

```cpp
using namespace omath::source_engine;

// Correct: uses Source Engine trait
Mesh my_mesh(vbo, vao);

// Avoid: manually specifying template parameters
primitives::Mesh<Mat4X4, ViewAngles, MeshTrait, float> verbose_mesh(vbo, vao);
```

---

## Notes

* **Angle ranges**: Ensure angles are within valid ranges (pitch: [-89°, 89°], yaw/roll: [-180°, 180°])
* **Performance**: Matrix computation is O(1) with ~64 floating-point operations
* **Caching**: The mesh caches the transformation matrix; recomputed only when rotation changes
* **Compatibility**: Works with all Source Engine games (CS:GO, TF2, Portal, Half-Life 2, etc.)

---

## See Also

- [Mesh Documentation](../../3d_primitives/mesh.md) - Mesh primitive using this trait
- [MeshCollider Documentation](../../collision/mesh_collider.md) - Collision wrapper for meshes
- [Formulas Documentation](formulas.md) - Source Engine rotation formula
- [CameraTrait Documentation](camera_trait.md) - Camera transformation trait
- [Constants Documentation](constants.md) - Source Engine constants
- [API Overview](../../api_overview.md) - High-level API reference

---

*Last updated: 13 Nov 2025*
