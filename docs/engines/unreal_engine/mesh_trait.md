# `omath::unreal_engine::MeshTrait` — mesh transformation trait for Unreal Engine

> Header: `omath/engines/unreal_engine/traits/mesh_trait.hpp`
> Namespace: `omath::unreal_engine`
> Purpose: provide Unreal Engine-specific rotation matrix computation for `omath::primitives::Mesh`

---

## Summary

`MeshTrait` is a trait class that provides the `rotation_matrix` function for transforming meshes in Unreal Engine's coordinate system. It serves as a template parameter to `omath::primitives::Mesh`, enabling engine-specific rotation behavior.

---

## Coordinate System

**Unreal Engine** uses:
* **Up axis**: +Z
* **Forward axis**: +X
* **Right axis**: +Y
* **Handedness**: Left-handed
* **Rotation order**: Roll (Y) → Pitch (X) → Yaw (Z)

---

## API

```cpp
namespace omath::unreal_engine {

class MeshTrait final {
public:
    [[nodiscard]]
    static Mat4X4 rotation_matrix(const ViewAngles& rotation);
};

} // namespace omath::unreal_engine
```

---

## Method: `rotation_matrix`

```cpp
static Mat4X4 rotation_matrix(const ViewAngles& rotation);
```

Computes a 4×4 rotation matrix from Unreal-style Euler angles.

**Parameters**:
* `rotation` — `ViewAngles` containing pitch, yaw, and roll angles

**Returns**: 4×4 rotation matrix suitable for mesh transformation

**Implementation**: Delegates to `unreal_engine::rotation_matrix(rotation)` defined in `formulas.hpp`.

---

## Usage

### With Mesh

```cpp
using namespace omath::unreal_engine;

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

---

## Rotation Conventions

Unreal uses a left-handed Z-up coordinate system:

1. **Roll** (rotation around Y-axis / right axis)
   * Positive roll rotates forward axis upward
   * Range: [-180°, 180°]

2. **Pitch** (rotation around X-axis / forward axis)
   * Positive pitch looks upward
   * Range: typically [-89°, 89°]

3. **Yaw** (rotation around Z-axis / up axis)
   * Positive yaw rotates clockwise when viewed from above (left-handed)
   * Range: [-180°, 180°]

**Note**: Unreal applies rotations in Roll-Pitch-Yaw order, different from most other engines.

---

## Type Alias

```cpp
namespace omath::unreal_engine {
    using Mesh = primitives::Mesh<Mat4X4, ViewAngles, MeshTrait, float>;
}
```

---

## See Also

- [Mesh Documentation](../../3d_primitives/mesh.md) - Mesh primitive
- [Formulas Documentation](formulas.md) - Unreal rotation formula
- [CameraTrait Documentation](camera_trait.md) - Camera trait

---

*Last updated: 13 Nov 2025*
