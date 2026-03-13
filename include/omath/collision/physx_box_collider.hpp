//
// Created by orange-cpp
//
#pragma once

#ifdef OMATH_ENABLE_PHYSX

#include "collider_interface.hpp"
#include <PxPhysicsAPI.h>

namespace omath::collision
{
    /// Axis-aligned box collider backed by PhysX PxBoxGeometry.
    /// Half-extents are stored in PhysX convention (positive values along each axis).
    class PhysXBoxCollider final : public ColliderInterface<Vector3<float>>
    {
    public:
        /// @param half_extents  Half-widths along X, Y and Z axes (all must be > 0).
        /// @param origin        World-space centre of the box.
        explicit PhysXBoxCollider(const VectorType& half_extents, const VectorType& origin = {})
            : m_geometry(physx::PxVec3(half_extents.x, half_extents.y, half_extents.z))
            , m_origin(origin)
        {
        }

        /// Support function: returns the world-space point on the box furthest in @p direction.
        /// For a box, the furthest point along d is origin + (sign(d.x)*hx, sign(d.y)*hy, sign(d.z)*hz).
        [[nodiscard]]
        VectorType find_abs_furthest_vertex_position(const VectorType& direction) const override
        {
            const auto& he = m_geometry.halfExtents;
            return {
                m_origin.x + (direction.x >= 0.f ? he.x : -he.x),
                m_origin.y + (direction.y >= 0.f ? he.y : -he.y),
                m_origin.z + (direction.z >= 0.f ? he.z : -he.z),
            };
        }

        [[nodiscard]]
        const VectorType& get_origin() const override { return m_origin; }

        void set_origin(const VectorType& new_origin) override { m_origin = new_origin; }

        [[nodiscard]]
        const physx::PxBoxGeometry& get_geometry() const { return m_geometry; }

        /// Update half-extents at runtime.
        void set_half_extents(const VectorType& half_extents)
        {
            m_geometry = physx::PxBoxGeometry(physx::PxVec3(half_extents.x, half_extents.y, half_extents.z));
        }

    private:
        physx::PxBoxGeometry m_geometry;
        VectorType m_origin;
    };
} // namespace omath::collision

#endif // OMATH_ENABLE_PHYSX
