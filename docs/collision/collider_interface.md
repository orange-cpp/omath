# `omath::collision::ColliderInterface` — Abstract collider base class

> Header: `omath/collision/collider_interface.hpp`
> Namespace: `omath::collision`
> Depends on: `omath/linear_algebra/vector3.hpp`

`ColliderInterface` is the abstract base class for all colliders used by the GJK and EPA algorithms. Any shape that can report its furthest vertex along a given direction can implement this interface and be used for collision detection.

---

## API

```cpp
namespace omath::collision {

template<class VecType = Vector3<float>>
class ColliderInterface {
public:
    using VectorType = VecType;

    virtual ~ColliderInterface() = default;

    // Return the world-space position of the vertex furthest along `direction`.
    [[nodiscard]]
    virtual VectorType find_abs_furthest_vertex_position(
        const VectorType& direction) const = 0;

    // Get the collider's origin (center / position).
    [[nodiscard]]
    virtual const VectorType& get_origin() const = 0;

    // Reposition the collider.
    virtual void set_origin(const VectorType& new_origin) = 0;
};

} // namespace omath::collision
```

---

## Implementing a custom collider

To create a new collider shape, derive from `ColliderInterface` and implement the three pure-virtual methods:

```cpp
#include "omath/collision/collider_interface.hpp"

class SphereCollider final
    : public omath::collision::ColliderInterface<omath::Vector3<float>>
{
public:
    SphereCollider(omath::Vector3<float> center, float radius)
        : m_center(center), m_radius(radius) {}

    [[nodiscard]]
    omath::Vector3<float> find_abs_furthest_vertex_position(
        const omath::Vector3<float>& direction) const override
    {
        return m_center + direction.normalized() * m_radius;
    }

    [[nodiscard]]
    const omath::Vector3<float>& get_origin() const override
    { return m_center; }

    void set_origin(const omath::Vector3<float>& new_origin) override
    { m_center = new_origin; }

private:
    omath::Vector3<float> m_center;
    float m_radius;
};
```

---

## Notes

* **Template parameter**: The default vector type is `Vector3<float>`, but any vector type with a `dot()` method can be used.
* **GJK/EPA compatibility**: Both `GjkAlgorithm` and `EpaAlgorithm` accept any type satisfying the `ColliderInterface` contract through their template parameters.

---

## See also

* [GJK Algorithm](gjk_algorithm.md) — collision detection using GJK.
* [EPA Algorithm](epa_algorithm.md) — penetration depth via EPA.
* [Mesh Collider](mesh_collider.md) — concrete collider wrapping a `Mesh`.
* [Simplex](simplex.md) — simplex data structure used by GJK/EPA.

---

*Last updated: Feb 2026*
