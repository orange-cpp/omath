# `omath::opengl_engine::MeshTrait` — mesh transformation trait for OpenGL

> Header: `omath/engines/opengl_engine/traits/mesh_trait.hpp`
> Namespace: `omath::opengl_engine`
> Purpose: provide OpenGL-specific rotation matrix computation for `omath::primitives::Mesh`

---

## Summary

`MeshTrait` is a trait class that provides the `rotation_matrix` function for transforming meshes in OpenGL's canonical coordinate system. It serves as a template parameter to `omath::primitives::Mesh`, enabling engine-specific rotation behavior.

---

## Coordinate System

**OpenGL** (canonical) uses:
* **Up axis**: +Y
* **Forward axis**: +Z (toward viewer)
* **Right axis**: +X
* **Handedness**: Right-handed
* **Rotation order**: Pitch (X) → Yaw (Y) → Roll (Z)

---

## API

```cpp
namespace omath::opengl_engine {

class MeshTrait final {
public:
    [[nodiscard]]
    static Mat4X4 rotation_matrix(const ViewAngles& rotation);
};

} // namespace omath::opengl_engine
```

---

## Method: `rotation_matrix`

```cpp
static Mat4X4 rotation_matrix(const ViewAngles& rotation);
```

Computes a 4×4 rotation matrix from OpenGL-style Euler angles.

**Parameters**:
* `rotation` — `ViewAngles` containing pitch, yaw, and roll angles

**Returns**: 4×4 rotation matrix suitable for mesh transformation

**Implementation**: Delegates to `opengl_engine::rotation_matrix(rotation)` defined in `formulas.hpp`.

---

## Usage

### With Mesh

```cpp
using namespace omath::opengl_engine;

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

OpenGL uses a right-handed Y-up coordinate system:

1. **Pitch** (rotation around X-axis / right axis)
   * Positive pitch looks upward (+Y direction)
   * Range: typically [-89°, 89°]

2. **Yaw** (rotation around Y-axis / up axis)
   * Positive yaw rotates counterclockwise when viewed from above (right-handed)
   * Range: [-180°, 180°]

3. **Roll** (rotation around Z-axis / depth axis)
   * Positive roll tilts right
   * Range: [-180°, 180°]

**Note**: In OpenGL, +Z points toward the viewer in view space, but away from the viewer in world space.

---

## Type Alias

```cpp
namespace omath::opengl_engine {
    using Mesh = primitives::Mesh<Mat4X4, ViewAngles, MeshTrait, float>;
}
```

---

## See Also

- [Mesh Documentation](../../3d_primitives/mesh.md) - Mesh primitive
- [Formulas Documentation](formulas.md) - OpenGL rotation formula
- [CameraTrait Documentation](camera_trait.md) - Camera trait

---

*Last updated: 13 Nov 2025*
