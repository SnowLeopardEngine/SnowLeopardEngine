#include "SnowLeopardEngine/Function/Physics/PhysicsSystem.h"
#include "PxActor.h"
#include "PxRigidActor.h"
#include "PxRigidDynamic.h"
#include "PxShape.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "entt/entity/fwd.hpp"
#include "foundation/Px.h"
#include "geometry/PxBoxGeometry.h"
#include "glm/fwd.hpp"

using namespace physx;

namespace SnowLeopardEngine
{
    // Set the actions when collision occurs,Physx needs to do.
    static PxFilterFlags SimulationFilterShader(PxFilterObjectAttributes attributes0,
                                                PxFilterData             filterData0,
                                                PxFilterObjectAttributes attributes1,
                                                PxFilterData             filterData1,
                                                PxPairFlags&             pairFlags,
                                                const void*              constantBlock,
                                                PxU32                    constantBlockSize)
    {
        pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND;
        return PxFilterFlags();
    }

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
        m_LogicScene = logicScene;
        // Create a scene
        PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
        sceneDesc.gravity                 = PxVec3(0.0f, -9.8f, 0.0f); // scene gravity
        auto* cpuDispatcher               = PxDefaultCpuDispatcherCreate(2);
        sceneDesc.cpuDispatcher           = cpuDispatcher;
        sceneDesc.simulationEventCallback = this;
        // sceneDesc.filterShader  = PxDefaultSimulationFilterShader;
        sceneDesc.filterShader = SimulationFilterShader;
        m_Scene                = m_Physics->createScene(sceneDesc);
        auto* pvdClient        = m_Scene->getScenePvdClient();
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
                    // TODO: Jubiao Lin setLinearDamping, setAngularDamping, enable ccd here:
                    // https://github.com/SnowLeopardEngine/SnowLeopardEngine/issues/10
                    body = m_Physics->createRigidDynamic(pxTransform);
                    static_cast<PxRigidDynamic*>(body)->setMass(rigidBody.Mass);
                    static_cast<PxRigidDynamic*>(body)->setLinearDamping(rigidBody.LinearDamping);
                    static_cast<PxRigidDynamic*>(body)->setAngularDamping(rigidBody.AngularDamping);

                    PxRigidBodyFlags currentFlags = static_cast<PxRigidDynamic*>(body)->getRigidBodyFlags();
                    if (rigidBody.EnableCCD)
                    {
                        currentFlags |= PxRigidBodyFlag::eENABLE_CCD;
                    }
                    else
                    {
                        currentFlags &= ~PxRigidBodyFlag::eENABLE_CCD;
                    }
                    static_cast<PxRigidDynamic*>(body)->setRigidBodyFlags(currentFlags);
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

                PxShapeFlags shapeFlags;
                if (sphereCollider.IsTrigger)
                {
                    shapeFlags = PxShapeFlag::eTRIGGER_SHAPE;
                }
                else
                {
                    shapeFlags = PxShapeFlag::eSIMULATION_SHAPE;
                }

                float radius = 0.5f;
                if (sphereCollider.Radius == 0)
                {
                    radius = radius * transform.Scale.x;
                }
                PxSphereGeometry sphereGeometry(radius);
                auto*            sphereShape = m_Physics->createShape(sphereGeometry, *material);

                // attach the shape to the rigidBody
                body->attachShape(*sphereShape);

                // add the rigidBody to the scene
                m_Scene->addActor(*body);

                rigidBody.InternalBody = body;
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
                    // TODO: Jubiao Lin setLinearDamping, setAngularDamping, enable ccd here:
                    // https://github.com/SnowLeopardEngine/SnowLeopardEngine/issues/10
                    body = m_Physics->createRigidDynamic(pxTransform);
                    static_cast<PxRigidDynamic*>(body)->setMass(rigidBody.Mass);
                    static_cast<PxRigidDynamic*>(body)->setLinearDamping(rigidBody.LinearDamping);
                    static_cast<PxRigidDynamic*>(body)->setAngularDamping(rigidBody.AngularDamping);

                    PxRigidBodyFlags currentFlags = static_cast<PxRigidDynamic*>(body)->getRigidBodyFlags();
                    if (rigidBody.EnableCCD)
                    {
                        currentFlags |= PxRigidBodyFlag::eENABLE_CCD;
                    }
                    else
                    {
                        currentFlags &= ~PxRigidBodyFlag::eENABLE_CCD;
                    }
                    static_cast<PxRigidDynamic*>(body)->setRigidBodyFlags(currentFlags);
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

                PxShapeFlags shapeFlags;
                if (boxCollider.IsTrigger)
                {
                    shapeFlags = PxShapeFlag::eTRIGGER_SHAPE;
                }
                else
                {
                    shapeFlags = PxShapeFlag::eSIMULATION_SHAPE;
                }

                glm::vec3 size = {1, 1, 1};
                if (boxCollider.Size == glm::vec3(0, 0, 0))
                {
                    size.x *= transform.Scale.x;
                    size.y *= transform.Scale.y;
                    size.z *= transform.Scale.z;
                }
                PxBoxGeometry boxGeometry(size.x / 2.0f, size.y / 2.0f, size.z / 2.0f);
                auto*         boxShape = m_Physics->createShape(boxGeometry, *material);

                // attach the shape to the rigidBody
                body->attachShape(*boxShape);

                // add the rigidBody to the scene
                m_Scene->addActor(*body);

                rigidBody.InternalBody = body;
            });

        // case3: RigidBodyComponent + CapsuleColliderComponent
        registry.view<TransformComponent, EntityStatusComponent, RigidBodyComponent, CapsuleColliderComponent>().each(
            [this](entt::entity              entity,
                   TransformComponent&       transform,
                   EntityStatusComponent&    entityStatus,
                   RigidBodyComponent&       rigidBody,
                   CapsuleColliderComponent& capsuleCollider) {
                // create a rigidBody
                PxTransform   pxTransform(transform.Position.x + capsuleCollider.Offset.x,
                                        transform.Position.y + capsuleCollider.Offset.y,
                                        transform.Position.z + capsuleCollider.Offset.z);
                PxRigidActor* body;
                if (entityStatus.IsStatic)
                {
                    body = m_Physics->createRigidStatic(pxTransform);
                }
                else
                {
                    body = m_Physics->createRigidDynamic(pxTransform);
                    static_cast<PxRigidDynamic*>(body)->setMass(rigidBody.Mass);
                    static_cast<PxRigidDynamic*>(body)->setLinearDamping(rigidBody.LinearDamping);
                    static_cast<PxRigidDynamic*>(body)->setAngularDamping(rigidBody.AngularDamping);

                    PxRigidBodyFlags currentFlags = static_cast<PxRigidDynamic*>(body)->getRigidBodyFlags();
                    if (rigidBody.EnableCCD)
                    {
                        currentFlags |= PxRigidBodyFlag::eENABLE_CCD;
                    }
                    else
                    {
                        currentFlags &= ~PxRigidBodyFlag::eENABLE_CCD;
                    }
                    static_cast<PxRigidDynamic*>(body)->setRigidBodyFlags(currentFlags);
                }

                // create a box shape
                PxMaterial* material;
                if (capsuleCollider.Material == nullptr)
                {
                    material = m_Physics->createMaterial(0.0f, 0.0f, 0.0f);
                }
                else
                {
                    material = m_Physics->createMaterial(capsuleCollider.Material->DynamicFriction,
                                                         capsuleCollider.Material->StaticFriction,
                                                         capsuleCollider.Material->Bounciness);
                }

                PxCapsuleGeometry capsuleGeometry(capsuleCollider.Radius, capsuleCollider.Height);
                auto*             capsuleShape = m_Physics->createShape(capsuleGeometry, *material);
                body->attachShape(*capsuleShape);
                m_Scene->addActor(*body);

                rigidBody.InternalBody = body;
            });

        // case4: Rigid + Heightfield
        //  registry.view<TransformComponent, EntityStatusComponent, RigidBodyComponent, HeightfieldColliderComponent,
        //  DampingComponent>().each(
        //      [this](entt::entity           entity,
        //             TransformComponent&    transform,
        //             EntityStatusComponent& entityStatus,
        //             RigidBodyComponent&    rigidBody,
        //             HeightfieldColliderComponent&  heightfieldCollider,
        //             DampingComponent&      damping) {

        //         auto t = heightfieldCollider.Width * heightfieldCollider.Height;
        //         PxHeightFieldSample* hfSample = new PxHeightFieldSample[t];
        //         for(unsigned i = 0; i<t; i++){
        //             hfSample[i].Height = static_cast<PxI16>(hfSample->height[i]);
        //             hfSample[i].materialIndex0 = hfSample[i].materialIndex1 = 0;
        //         }

        //         PxHeightFieldDesc hfDesc;
        //         hfDesc.format = PxHeightFieldFormat::eS16_TM;
        //         hfDesc.nbColumns = hfSample.Width;
        //         hfDesc.nbRows = hfSample.Height;
        //         hfDesc.samples.data = hfSample;
        //         hfDesc.samples.stride = sizeof(PxHeightFieldSample);

        //         PxHeightField* heightField = m_Physics->createHeightField(hfDesc);
        //         delete[] hfSample;

        //         PxHeightFieldGeometry hfGeometry(heightField, PxMeshGeometryFlags(), 1.0f, 1.0f, 1.0f);
        //         PxMaterial* material = m_Physics->createMaterial(heightfieldCollider.Material->DynamicFriction,
        //                                                          heightfieldCollider.Material->StaticFriction,
        //                                                          heightfieldCollider.Material->Bounciness);

        //         PxRigidActor* body;
        //         if(entityStatus.IsStatic){
        //             body = m_Physics->createRigidStatic(PxTransform(FromGlmVec3(transform.Position)));
        //         }

        //         body->createShape(hfGeometry, *material);
        //         m_Scene->addActor(*body);

        //            });
        // TODO: More cases
    }

    void PhysicsSystem::OnFixedTick()
    {
        if (m_Scene != nullptr)
        {
            m_Scene->simulate(Time::FixedDeltaTime);
            m_Scene->fetchResults(true);

            auto& registry = m_LogicScene->GetRegistry();
            registry.view<TransformComponent, RigidBodyComponent>().each(
                [](entt::entity entity, TransformComponent& transform, RigidBodyComponent& rigidBody) {
                    PxTransform pxTransform = rigidBody.InternalBody->getGlobalPose();
                    PxVec3      pxPosition  = pxTransform.p;
                    PxQuat      pxRotation  = pxTransform.q;
                    transform.Position      = {pxPosition.x, pxPosition.y, pxPosition.z};
                    glm::quat rotation      = {pxRotation.w, pxRotation.x, pxRotation.y, pxRotation.z};
                    transform.SetRotation(rotation);
                });
        }
    }

    void PhysicsSystem::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
    {
        SNOW_LEOPARD_CORE_INFO("[PhysicsSystem][EventCallBack] onConstraintBreak");
    }

    void PhysicsSystem::onWake(physx::PxActor** actors, physx::PxU32 count)
    {
        SNOW_LEOPARD_CORE_INFO("[PhysicsSystem][EventCallBack] onWake");
    }

    void PhysicsSystem::onSleep(physx::PxActor** actors, physx::PxU32 count)
    {
        SNOW_LEOPARD_CORE_INFO("[PhysicsSystem][EventCallBack] onSleep");
    }

    void PhysicsSystem::onContact(const physx::PxContactPairHeader& pairHeader,
                                  const physx::PxContactPair*       pairs,
                                  physx::PxU32                      count)
    {
        SNOW_LEOPARD_CORE_INFO("[PhysicsSystem][EventCallBack] onContact");
        if (m_LogicScene != nullptr)
        {
            auto& registry = m_LogicScene->GetRegistry();
            registry.view<NativeScriptingComponent>().each(
                [](entt::entity entity, NativeScriptingComponent& nativescripting) {
                    nativescripting.ScriptInstance->OnColliderEnter();
                });
        }
    }

    void PhysicsSystem::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
    {
        SNOW_LEOPARD_CORE_INFO("[PhysicsSystem][EventCallBack] onTrigger");
    }

    void PhysicsSystem::onAdvance(const physx::PxRigidBody* const* bodyBuffer,
                                  const physx::PxTransform*        poseBuffer,
                                  const physx::PxU32               count)
    {
        SNOW_LEOPARD_CORE_INFO("[PhysicsSystem][EventCallBack] onAdvance");
    }
} // namespace SnowLeopardEngine