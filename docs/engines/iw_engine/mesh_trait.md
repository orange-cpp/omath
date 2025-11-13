# `omath::iw_engine::MeshTrait` — mesh transformation trait for IW Engine

> Header: `omath/engines/iw_engine/traits/mesh_trait.hpp`
> Namespace: `omath::iw_engine`
> Purpose: provide IW Engine-specific rotation matrix computation for `omath::primitives::Mesh`

---

## Summary

`MeshTrait` is a trait class that provides the `rotation_matrix` function for transforming meshes in IW Engine's (Infinity Ward) coordinate system. It serves as a template parameter to `omath::primitives::Mesh`, enabling engine-specific rotation behavior.

---

## Coordinate System

**IW Engine** (Call of Duty) uses:
* **Up axis**: +Z
* **Forward axis**: +Y
* **Right axis**: +X
* **Handedness**: Right-handed
* **Rotation order**: Pitch (X) → Yaw (Z) → Roll (Y)

---

## API

```cpp
namespace omath::iw_engine {

class MeshTrait final {
public:
    [[nodiscard]]
    static Mat4X4 rotation_matrix(const ViewAngles& rotation);
};

} // namespace omath::iw_engine
```

---

## Method: `rotation_matrix`

```cpp
static Mat4X4 rotation_matrix(const ViewAngles& rotation);
```

Computes a 4×4 rotation matrix from IW Engine-style Euler angles.

**Parameters**:
* `rotation` — `ViewAngles` containing pitch, yaw, and roll angles

**Returns**: 4×4 rotation matrix suitable for mesh transformation

**Implementation**: Delegates to `iw_engine::rotation_matrix(rotation)` defined in `formulas.hpp`.

---

## Usage

### With Mesh

```cpp
using namespace omath::iw_engine;

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

IW Engine uses a right-handed Z-up coordinate system (similar to Source Engine):

1. **Pitch** (rotation around X-axis / right axis)
   * Positive pitch looks upward (+Z direction)
   * Range: typically [-89°, 89°]

2. **Yaw** (rotation around Z-axis / up axis)
   * Positive yaw rotates counterclockwise when viewed from above
   * Range: [-180°, 180°]

3. **Roll** (rotation around Y-axis / forward axis)
   * Positive roll tilts right
   * Range: [-180°, 180°]

---

## Type Alias

```cpp
namespace omath::iw_engine {
    using Mesh = primitives::Mesh<Mat4X4, ViewAngles, MeshTrait, float>;
}
```

---

## See Also

- [Mesh Documentation](../../3d_primitives/mesh.md) - Mesh primitive
- [Formulas Documentation](formulas.md) - IW Engine rotation formula
- [CameraTrait Documentation](camera_trait.md) - Camera trait

---

*Last updated: 13 Nov 2025*
