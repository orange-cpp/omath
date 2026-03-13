//
// Created by orange-cpp
//
#pragma once

#ifdef OMATH_ENABLE_PHYSX

#include <PxPhysicsAPI.h>

namespace omath::collision
{
    /// RAII owner of a PhysX Foundation + Physics + Scene.
    /// One world per simulation context; not copyable or movable.
    class PhysXWorld final
    {
    public:
        explicit PhysXWorld(physx::PxVec3 gravity = {0.f, -9.81f, 0.f},
                            physx::PxU32 cpu_threads = 2)
        {
            m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_error_callback);

            m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation,
                                        physx::PxTolerancesScale{});

            physx::PxSceneDesc desc(m_physics->getTolerancesScale());
            desc.gravity       = gravity;
            desc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(cpu_threads);
            m_dispatcher       = static_cast<physx::PxDefaultCpuDispatcher*>(desc.cpuDispatcher);
            desc.filterShader  = physx::PxDefaultSimulationFilterShader;

            m_scene = m_physics->createScene(desc);

            // Default material: static friction 0.5, dynamic friction 0.5, restitution 0.
            m_default_material = m_physics->createMaterial(0.5f, 0.5f, 0.f);
        }

        ~PhysXWorld()
        {
            m_scene->release();
            m_dispatcher->release();
            m_default_material->release();
            m_physics->release();
            m_foundation->release();
        }

        PhysXWorld(const PhysXWorld&)            = delete;
        PhysXWorld& operator=(const PhysXWorld&) = delete;

        /// Advance the simulation by @p dt seconds and block until results are ready.
        void step(float dt)
        {
            m_scene->simulate(dt);
            m_scene->fetchResults(true);
        }

        [[nodiscard]] physx::PxPhysics&  get_physics()          { return *m_physics;          }
        [[nodiscard]] physx::PxScene&    get_scene()             { return *m_scene;             }
        [[nodiscard]] physx::PxMaterial& get_default_material()  { return *m_default_material;  }

        /// Add an infinite static ground plane at y = @p y_level facing +Y.
        physx::PxRigidStatic* add_ground_plane(float y_level = 0.f)
        {
            physx::PxRigidStatic* plane = PxCreatePlane(
                *m_physics,
                physx::PxPlane(0.f, 1.f, 0.f, -y_level),
                *m_default_material);
            m_scene->addActor(*plane);
            return plane;
        }

    private:
        physx::PxDefaultAllocator      m_allocator{};
        physx::PxDefaultErrorCallback  m_error_callback{};
        physx::PxFoundation*           m_foundation{nullptr};
        physx::PxPhysics*              m_physics{nullptr};
        physx::PxDefaultCpuDispatcher* m_dispatcher{nullptr};
        physx::PxScene*                m_scene{nullptr};
        physx::PxMaterial*             m_default_material{nullptr};
    };
} // namespace omath::collision

#endif // OMATH_ENABLE_PHYSX
