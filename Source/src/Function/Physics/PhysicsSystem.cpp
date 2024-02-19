#include "SnowLeopardEngine/Function/Physics/PhysicsSystem.h"
#include "PxActor.h"
#include "PxRigidActor.h"
#include "PxRigidDynamic.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "geometry/PxBoxGeometry.h"

using namespace physx;

namespace SnowLeopardEngine
{
    PhysicsSystem::PhysicsSystem()
    {
        // Create foundation
        m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
        if (!m_Foundation)
        {
            SNOW_LEOPARD_CORE_ERROR("[PhysicsSystem] PxCreateFoundation failed!");
            m_State = SystemState::Error;
            return;
        }

        // Create pvd (debugging handle)
        auto* const pvd = PxCreatePvd(*m_Foundation);

        // Create pvd session, connect.
        auto* const transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
        pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

        // Create physics
        m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), true, pvd);
        if (!m_Physics)
        {
            SNOW_LEOPARD_CORE_ERROR("[PhysicsSystem] PxCreatePhysics failed!");
            m_State = SystemState::Error;
            return;
        }

        SNOW_LEOPARD_CORE_INFO("[PhysicsSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    PhysicsSystem::~PhysicsSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[PhysicsSystem] Shutting Down...");

        if (m_Scene != nullptr)
        {
            m_Scene->release();
        }
        m_Physics->release();
        m_Foundation->release();

        m_State = SystemState::ShutdownOk;
    }

    void PhysicsSystem::CookPhysicsScene(const Ref<LogicScene>& logicScene)
    {
        // Create a scene
        PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
        sceneDesc.gravity       = PxVec3(0.0f, -9.8f, 0.0f); // scene gravity
        auto* cpuDispatcher     = PxDefaultCpuDispatcherCreate(2);
        sceneDesc.cpuDispatcher = cpuDispatcher;
        sceneDesc.filterShader  = PxDefaultSimulationFilterShader;
        m_Scene                 = m_Physics->createScene(sceneDesc);
        auto* pvdClient         = m_Scene->getScenePvdClient();
        if (pvdClient)
        {
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
        }

        // Cook the scene from logic
        auto& registry = logicScene->GetRegistry();

        // Case 1: RigidBody + SphereCollider
        registry.view<TransformComponent, EntityStatusComponent, RigidBodyComponent, SphereColliderComponent>().each(
            [this](entt::entity             entity,
                   TransformComponent&      transform,
                   EntityStatusComponent&   entityStatus,
                   RigidBodyComponent&      rigidBody,
                   SphereColliderComponent& sphereCollider) {
                // create a rigidBody
                PxTransform   pxTransform(transform.Position.x, transform.Position.y, transform.Position.z);
                PxRigidActor* body;
                if (entityStatus.IsStatic)
                {
                    body = m_Physics->createRigidStatic(pxTransform);
                }
                else
                {
                    body = m_Physics->createRigidDynamic(pxTransform);
                    static_cast<PxRigidDynamic*>(body)->setMass(rigidBody.Mass);
                }

                // create a sphere shape
                PxMaterial* material;
                if (sphereCollider.Material == nullptr)
                {
                    material = m_Physics->createMaterial(0.0f, 0.0f, 0.0f);
                }
                else
                {
                    material = m_Physics->createMaterial(sphereCollider.Material->DynamicFriction,
                                                         sphereCollider.Material->StaticFriction,
                                                         sphereCollider.Material->Bounciness);
                }
                PxSphereGeometry sphereGeometry(sphereCollider.Radius);
                // TODO: Simiao Wang Consider shape trigger flag here. read from sphereCollider.IsTrigger.
                auto*            sphereShape = m_Physics->createShape(sphereGeometry, *material);

                // attach the shape to the rigidBody
                body->attachShape(*sphereShape);

                // add the rigidBody to the scene
                m_Scene->addActor(*body);
            });

        // Case 2: RigidBody + BoxCollider
        registry.view<TransformComponent, EntityStatusComponent, RigidBodyComponent, BoxColliderComponent>().each(
            [this](entt::entity           entity,
                   TransformComponent&    transform,
                   EntityStatusComponent& entityStatus,
                   RigidBodyComponent&    rigidBody,
                   BoxColliderComponent&  boxCollider) {
                // create a rigidBody
                PxTransform   pxTransform(transform.Position.x + boxCollider.Offset.x,
                                        transform.Position.y + boxCollider.Offset.y,
                                        transform.Position.z + boxCollider.Offset.z);
                PxRigidActor* body;
                if (entityStatus.IsStatic)
                {
                    body = m_Physics->createRigidStatic(pxTransform);
                }
                else
                {
                    body = m_Physics->createRigidDynamic(pxTransform);
                    dynamic_cast<PxRigidDynamic*>(body)->setMass(rigidBody.Mass);
                }

                // create a box shape
                PxMaterial* material;
                if (boxCollider.Material == nullptr)
                {
                    material = m_Physics->createMaterial(0.0f, 0.0f, 0.0f);
                }
                else
                {
                    material = m_Physics->createMaterial(boxCollider.Material->DynamicFriction,
                                                         boxCollider.Material->StaticFriction,
                                                         boxCollider.Material->Bounciness);
                }
                PxBoxGeometry boxGeometry(
                    boxCollider.Size.x / 2.0f, boxCollider.Size.y / 2.0f, boxCollider.Size.z / 2.0f);
                // TODO: Simiao Wang Consider shape trigger flag here. read from boxCollider.IsTrigger.
                auto* boxShape = m_Physics->createShape(boxGeometry, *material);

                // attach the shape to the rigidBody
                body->attachShape(*boxShape);

                // add the rigidBody to the scene
                m_Scene->addActor(*body);
            });

        // TODO: More cases
    }

    void PhysicsSystem::OnFixedTick()
    {
        if (m_Scene != nullptr)
        {
            m_Scene->simulate(Time::FixedDeltaTime);
            m_Scene->fetchResults(true);
        }
    }
} // namespace SnowLeopardEngine