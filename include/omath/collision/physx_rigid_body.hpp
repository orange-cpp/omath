//
// Created by orange-cpp
//
#pragma once

#ifdef OMATH_ENABLE_PHYSX

#include "collider_interface.hpp"
#include "physx_world.hpp"
#include <PxPhysicsAPI.h>
#include <extensions/PxRigidBodyExt.h>
#include <cmath>

namespace omath::collision
{
    /// Dynamic rigid body backed by a PhysX PxRigidDynamic actor.
    /// Implements ColliderInterface so it can participate in both omath GJK
    /// and PhysX simulation-based collision resolution.
    ///
    /// Ownership: the actor is added to the world's scene on construction
    /// and removed + released on destruction.
    class PhysXRigidBody final : public ColliderInterface<Vector3<float>>
    {
    public:
        /// @param world     PhysXWorld that owns the scene.
        /// @param geometry  Shape geometry (PxBoxGeometry, PxSphereGeometry, …).
        /// @param origin    Initial world-space position.
        /// @param density   Mass density used to compute mass and inertia.
        PhysXRigidBody(PhysXWorld& world, const physx::PxGeometry& geometry,
                       const VectorType& origin = {}, float density = 1.f)
            : m_world(world)
            , m_geometry(geometry)
        {
            const physx::PxTransform pose(physx::PxVec3(origin.x, origin.y, origin.z));
            m_actor = world.get_physics().createRigidDynamic(pose);

            physx::PxShape* shape = world.get_physics().createShape(
                geometry, world.get_default_material(), true);
            m_actor->attachShape(*shape);
            shape->release();

            physx::PxRigidBodyExt::updateMassAndInertia(*m_actor, density);
            world.get_scene().addActor(*m_actor);
        }

        ~PhysXRigidBody() override
        {
            m_world.get_scene().removeActor(*m_actor);
            m_actor->release();
        }

        PhysXRigidBody(const PhysXRigidBody&)            = delete;
        PhysXRigidBody& operator=(const PhysXRigidBody&) = delete;

        // ── ColliderInterface ────────────────────────────────────────────────

        /// Support function — delegates to the stored geometry type so the body
        /// can be used with omath GJK alongside the non-simulated colliders.
        [[nodiscard]]
        VectorType find_abs_furthest_vertex_position(const VectorType& direction) const override
        {
            const VectorType o = get_origin();
            switch (m_geometry.getType())
            {
                case physx::PxGeometryType::eBOX:
                {
                    const auto& he = m_geometry.box().halfExtents;
                    return {
                        o.x + (direction.x >= 0.f ? he.x : -he.x),
                        o.y + (direction.y >= 0.f ? he.y : -he.y),
                        o.z + (direction.z >= 0.f ? he.z : -he.z),
                    };
                }
                case physx::PxGeometryType::eSPHERE:
                {
                    const float r   = m_geometry.sphere().radius;
                    const float len = std::sqrt(direction.x * direction.x +
                                               direction.y * direction.y +
                                               direction.z * direction.z);
                    if (len == 0.f)
                        return o;
                    const float inv = r / len;
                    return { o.x + direction.x * inv,
                             o.y + direction.y * inv,
                             o.z + direction.z * inv };
                }
                default:
                    return o; // unsupported geometry — return centre
            }
        }

        [[nodiscard]]
        const VectorType& get_origin() const override
        {
            const auto& p = m_actor->getGlobalPose().p;
            m_cached_origin = { p.x, p.y, p.z };
            return m_cached_origin;
        }

        void set_origin(const VectorType& new_origin) override
        {
            physx::PxTransform pose = m_actor->getGlobalPose();
            pose.p = physx::PxVec3(new_origin.x, new_origin.y, new_origin.z);
            m_actor->setGlobalPose(pose);
        }

        // ── PhysX-specific API ───────────────────────────────────────────────

        [[nodiscard]] physx::PxRigidDynamic& get_actor() { return *m_actor; }
        [[nodiscard]] const physx::PxRigidDynamic& get_actor() const { return *m_actor; }

        void set_linear_velocity(const VectorType& v)
        {
            m_actor->setLinearVelocity(physx::PxVec3(v.x, v.y, v.z));
        }

        [[nodiscard]]
        VectorType get_linear_velocity() const
        {
            const auto& v = m_actor->getLinearVelocity();
            return { v.x, v.y, v.z };
        }

        void set_kinematic(bool enabled)
        {
            m_actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, enabled);
        }

    private:
        PhysXWorld&                    m_world;
        physx::PxGeometryHolder        m_geometry;
        physx::PxRigidDynamic*         m_actor{nullptr};
        mutable VectorType             m_cached_origin{};
    };
} // namespace omath::collision

#endif // OMATH_ENABLE_PHYSX
