//
// Created by orange-cpp
//
#pragma once

#ifdef OMATH_ENABLE_PHYSX

#include "collider_interface.hpp"
#include <PxPhysicsAPI.h>
#include <cmath>

namespace omath::collision
{
    /// Sphere collider backed by PhysX PxSphereGeometry.
    class PhysXSphereCollider final : public ColliderInterface<Vector3<float>>
    {
    public:
        /// @param radius  Sphere radius (must be > 0).
        /// @param origin  World-space centre of the sphere.
        explicit PhysXSphereCollider(float radius, const VectorType& origin = {})
            : m_geometry(radius)
            , m_origin(origin)
        {
        }

        /// Support function: returns the world-space point on the sphere furthest in @p direction.
        /// For a sphere that is simply origin + normalize(direction) * radius.
        [[nodiscard]]
        VectorType find_abs_furthest_vertex_position(const VectorType& direction) const override
        {
            const float len = std::sqrt(direction.x * direction.x +
                                        direction.y * direction.y +
                                        direction.z * direction.z);
            if (len == 0.f)
                return m_origin;

            const float inv = m_geometry.radius / len;
            return {
                m_origin.x + direction.x * inv,
                m_origin.y + direction.y * inv,
                m_origin.z + direction.z * inv,
            };
        }

        [[nodiscard]]
        const VectorType& get_origin() const override { return m_origin; }

        void set_origin(const VectorType& new_origin) override { m_origin = new_origin; }

        [[nodiscard]]
        const physx::PxSphereGeometry& get_geometry() const { return m_geometry; }

        [[nodiscard]]
        float get_radius() const { return m_geometry.radius; }

        void set_radius(float radius) { m_geometry = physx::PxSphereGeometry(radius); }

    private:
        physx::PxSphereGeometry m_geometry;
        VectorType m_origin;
    };
} // namespace omath::collision

#endif // OMATH_ENABLE_PHYSX
