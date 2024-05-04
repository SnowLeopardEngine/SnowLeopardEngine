#include "SnowLeopardEngine/Function/Physics/PhysicsSystem.h"
#include "PxActorData.h"
#include "PxForceMode.h"
#include "PxRigidBody.h"
#include "PxRigidDynamic.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Util/Util.h"

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

        // Create cooking
        m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, PxCookingParams(PxTolerancesScale()));
        if (!m_Cooking)
        {
            SNOW_LEOPARD_CORE_ERROR("[PhysicsSystem] PxCreateCooking failed!");
            m_State = SystemState::Error;
            return;
        }

        Subscribe(m_LogicSceneLoadedHandler);

        SNOW_LEOPARD_CORE_INFO("[PhysicsSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    PhysicsSystem::~PhysicsSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[PhysicsSystem] Shutting Down...");

        Unsubscribe(m_LogicSceneLoadedHandler);

        // TODO: When scene unloading
        ReleaseInternalResources();

        if (m_ControllerManager != nullptr)
        {
            m_ControllerManager->release();
        }

        if (m_Scene != nullptr)
        {
            m_Scene->release();
        }

        m_Cooking->release();
        m_Physics->release();
        m_Foundation->release();

        m_State = SystemState::ShutdownOk;
    }

    void PhysicsSystem::CookPhysicsScene(LogicScene* logicScene)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        m_LogicScene = logicScene;

        if (m_ControllerManager)
        {
            m_ControllerManager->release();
            m_ControllerManager = nullptr;
        }

        if (m_Scene)
        {
            m_Scene->release();
            m_Scene = nullptr;
        }

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

        // Create Controller Manager
        m_ControllerManager = PxCreateControllerManager(*m_Scene);
        if (!m_ControllerManager)
        {
            SNOW_LEOPARD_CORE_ERROR("[PhysicsSystem] PxCreateControllerManager failed!");
            m_State = SystemState::Error;
            return;
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
                PxTransform pxTransform = PhysXGLMHelpers::GetPhysXTransform(&transform);

                rigidBody.IsStatic = entityStatus.IsStatic;

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

                if (sphereCollider.Radius == 0)
                {
                    sphereCollider.Radius = 0.5f * transform.Scale.x;
                }
                PxSphereGeometry sphereGeometry(sphereCollider.Radius);
                auto*            sphereShape = m_Physics->createShape(sphereGeometry, *material);

                // attach the shape to the rigidBody
                if (body->attachShape(*sphereShape))
                {
                    sphereShape->release();
                    sphereShape = nullptr;
                }

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
                PxTransform pxTransform = PhysXGLMHelpers::GetPhysXTransform(&transform);
                pxTransform.p += PhysXGLMHelpers::GetPhysXVec3(boxCollider.Offset);

                rigidBody.IsStatic = entityStatus.IsStatic;

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

                if (boxCollider.Size == glm::vec3(0, 0, 0))
                {
                    boxCollider.Size = transform.Scale;
                }
                PxBoxGeometry boxGeometry(
                    boxCollider.Size.x / 2.0f, boxCollider.Size.y / 2.0f, boxCollider.Size.z / 2.0f);
                auto* boxShape = m_Physics->createShape(boxGeometry, *material);

                // attach the shape to the rigidBody
                if (body->attachShape(*boxShape))
                {
                    boxShape->release();
                    boxShape = nullptr;
                }

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
                PxTransform pxTransform = PhysXGLMHelpers::GetPhysXTransform(&transform);
                pxTransform.p += PhysXGLMHelpers::GetPhysXVec3(capsuleCollider.Offset);

                rigidBody.IsStatic = entityStatus.IsStatic;

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
                if (body->attachShape(*capsuleShape))
                {
                    capsuleShape->release();
                    capsuleShape = nullptr;
                }

                m_Scene->addActor(*body);

                rigidBody.InternalBody = body;
            });

        // case4: TerrainComponent + TerrainColliderComponent
        registry.view<TransformComponent, EntityStatusComponent, TerrainComponent, TerrainColliderComponent>().each(
            [this](entt::entity              entity,
                   TransformComponent&       transform,
                   EntityStatusComponent&    entityStatus,
                   TerrainComponent&         terrain,
                   TerrainColliderComponent& terrainCollider) {
                uint32_t             heightMapWidth  = terrain.TerrainHeightMap.Width;
                uint32_t             heightMapHeight = terrain.TerrainHeightMap.Height;
                PxHeightFieldSample* hfSample        = new PxHeightFieldSample[heightMapWidth * heightMapHeight];

                for (uint32_t row = 0; row < heightMapHeight; ++row)
                {
                    for (uint32_t column = 0; column < heightMapWidth; ++column)
                    {
                        uint32_t sampleIndex = row + column * heightMapHeight;
                        // reverse because PhysX row = our column.
                        hfSample[sampleIndex].height = static_cast<PxI16>(terrain.TerrainHeightMap.Get(row, column));
                        hfSample[sampleIndex].materialIndex0 = hfSample[sampleIndex].materialIndex1 = 0;
                    }
                }

                PxHeightFieldDesc hfDesc;
                hfDesc.format         = PxHeightFieldFormat::eS16_TM;
                hfDesc.nbColumns      = heightMapWidth;
                hfDesc.nbRows         = heightMapHeight;
                hfDesc.samples.data   = hfSample;
                hfDesc.samples.stride = sizeof(PxHeightFieldSample);

                PxHeightField* heightField = PxCreateHeightField(hfDesc, m_Physics->getPhysicsInsertionCallback());
                delete[] hfSample;

                // reverse X-Z because PhysX row = our column.
                PxHeightFieldGeometry hfGeometry(
                    heightField, PxMeshGeometryFlags(), terrain.YScale, terrain.XScale, terrain.ZScale);
                PxMaterial* material;
                if (terrainCollider.Material == nullptr)
                {
                    material = m_Physics->createMaterial(0.0f, 0.0f, 0.0f);
                }
                else
                {
                    material = m_Physics->createMaterial(terrainCollider.Material->DynamicFriction,
                                                         terrainCollider.Material->StaticFriction,
                                                         terrainCollider.Material->Bounciness);
                }

                PxRigidStatic* body =
                    m_Physics->createRigidStatic(PxTransform(PhysXGLMHelpers::GetPhysXVec3(transform.Position)));

                PxShape* shape = m_Physics->createShape(hfGeometry, *material);
                if (body->attachShape(*shape))
                {
                    shape->release();
                    shape = nullptr;
                }

                m_Scene->addActor(*body);

                terrainCollider.InternalBody = body;
            });

        // case5: CharacterController
        registry.view<TransformComponent, CharacterControllerComponent>().each(
            [this](
                entt::entity entity, TransformComponent& transform, CharacterControllerComponent& characterController) {
                PxMaterial* material;
                if (characterController.Material == nullptr)
                {
                    material = m_Physics->createMaterial(0.0f, 0.0f, 0.0f);
                }
                else
                {
                    material = m_Physics->createMaterial(characterController.Material->DynamicFriction,
                                                         characterController.Material->StaticFriction,
                                                         characterController.Material->Bounciness);
                }

                // Set Controller Descripoter
                PxCapsuleControllerDesc desc;
                desc.height   = characterController.Height;
                desc.radius   = characterController.Radius;
                desc.material = material;
                desc.position.set(transform.Position.x + characterController.Offset.x,
                                  transform.Position.y + characterController.Offset.y,
                                  transform.Position.z + characterController.Offset.z);
                desc.upDirection = PxVec3(0, 1, 0);
                desc.slopeLimit  = cosf(characterController.SlopeLimit);
                desc.stepOffset  = characterController.StepOffset;
                desc.material    = material;

                PxController* controller = m_ControllerManager->createController(desc);

                characterController.InternalController = controller;
            });

        // case6: MeshFilterComponent + MeshColliderComponent + RigidBodyComponent
        registry
            .view<TransformComponent,
                  EntityStatusComponent,
                  RigidBodyComponent,
                  MeshFilterComponent,
                  MeshColliderComponent>()
            .each([this](entt::entity           entity,
                         TransformComponent&    transform,
                         EntityStatusComponent& entityStatus,
                         RigidBodyComponent&    rigidBody,
                         MeshFilterComponent&   meshFilter,
                         MeshColliderComponent& meshCollider) {
                PxTransform pxTransform = PhysXGLMHelpers::GetPhysXTransform(&transform);
                pxTransform.p += PhysXGLMHelpers::GetPhysXVec3(meshCollider.Offset);

                rigidBody.IsStatic = entityStatus.IsStatic;

                PxRigidActor* body;
                if (entityStatus.IsStatic)
                {
                    body = m_Physics->createRigidStatic(pxTransform);
                }
                else
                {
                    body = m_Physics->createRigidDynamic(pxTransform);
                    // static_cast<PxRigidDynamic*>(body)->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
                    static_cast<PxRigidDynamic*>(body)->setMass(rigidBody.Mass);                   // set Mass
                    static_cast<PxRigidDynamic*>(body)->setLinearDamping(rigidBody.LinearDamping); // set Linear Damping
                    static_cast<PxRigidDynamic*>(body)->setAngularDamping(
                        rigidBody.AngularDamping); // set Angular Damping

                    PxRigidBodyFlags currentFlags = static_cast<PxRigidDynamic*>(body)->getRigidBodyFlags();
                    if (rigidBody.EnableCCD) // Continuous Collision Detection
                    {
                        currentFlags |= PxRigidBodyFlag::eENABLE_CCD;
                    }
                    else
                    {
                        currentFlags &= ~PxRigidBodyFlag::eENABLE_CCD;
                    }
                    static_cast<PxRigidDynamic*>(body)->setRigidBodyFlags(currentFlags);
                }
                // create a mesh collider shape
                PxMaterial* material;
                if (meshCollider.Material == nullptr)
                {
                    material = m_Physics->createMaterial(0.0f, 0.0f, 0.0f);
                }
                else
                {
                    material = m_Physics->createMaterial(meshCollider.Material->DynamicFriction,
                                                         meshCollider.Material->StaticFriction,
                                                         meshCollider.Material->Bounciness);
                }

                PxShapeFlags shapeFlags;
                if (meshCollider.IsTrigger)
                {
                    shapeFlags = PxShapeFlag::eTRIGGER_SHAPE;
                }
                else
                {
                    shapeFlags = PxShapeFlag::eSIMULATION_SHAPE;
                }

                size_t totalVertexCount = 0;
                for (const auto& meshItem : meshFilter.Meshes->Items)
                {
                    totalVertexCount += meshItem.Data.Vertices.size();
                }

                PxVec3* vertices     = new PxVec3[totalVertexCount];
                size_t  vertexOffset = 0;
                for (const auto& meshItem : meshFilter.Meshes->Items)
                {
                    for (const auto& vertex : meshItem.Data.Vertices)
                    {
                        vertices[vertexOffset++] = PxVec3(vertex.Position.x, vertex.Position.y, vertex.Position.z);
                    }
                }

                // Switch to dynamic body by attach another convex mesh shape
                PxConvexMeshDesc convexDesc;
                convexDesc.points.count  = totalVertexCount;
                convexDesc.points.stride = sizeof(PxVec3);
                convexDesc.points.data   = vertices;
                convexDesc.flags         = PxConvexFlag::eCOMPUTE_CONVEX;

                PxDefaultMemoryOutputStream     writeBuffer;
                PxConvexMeshCookingResult::Enum result;
                m_Cooking->cookConvexMesh(convexDesc, writeBuffer, &result);

                PxDefaultMemoryInputData input(writeBuffer.getData(), writeBuffer.getSize());
                PxConvexMesh*            convexMesh = m_Physics->createConvexMesh(input);

                PxRigidActorExt::createExclusiveShape(*body, PxConvexMeshGeometry(convexMesh), *material);
                m_Scene->addActor(*body);

                rigidBody.InternalBody = body;

                delete[] vertices;
            });

        // TODO: More cases
    }

    void PhysicsSystem::OnFixedTick()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        if (m_Scene != nullptr)
        {
            auto& registry = m_LogicScene->GetRegistry();

            // Tick rigid bodies
            registry.view<TransformComponent, EntityStatusComponent, RigidBodyComponent>().each(
                [&registry](entt::entity           entity,
                            TransformComponent&    transform,
                            EntityStatusComponent& entityStatus,
                            RigidBodyComponent&    rigidBody) {
                    rigidBody.IsStatic = entityStatus.IsStatic;
                    if (rigidBody.InternalBody == nullptr)
                    {
                        auto name = registry.get<NameComponent>(entity).Name;
                        SNOW_LEOPARD_CORE_WARN("[PhysicsSystem] A rigidBody without shape! Entity name: {0}", name);
                        return;
                    }

                    if (!rigidBody.IsStatic)
                    {
                        // TODO: wrap as a function
                        static_cast<PxRigidDynamic*>(rigidBody.InternalBody)->setMass(rigidBody.Mass);
                        static_cast<PxRigidDynamic*>(rigidBody.InternalBody)->setLinearDamping(rigidBody.LinearDamping);
                        static_cast<PxRigidDynamic*>(rigidBody.InternalBody)
                            ->setAngularDamping(rigidBody.AngularDamping);

                        PxRigidBodyFlags currentFlags =
                            static_cast<PxRigidDynamic*>(rigidBody.InternalBody)->getRigidBodyFlags();
                        if (rigidBody.EnableCCD)
                        {
                            currentFlags |= PxRigidBodyFlag::eENABLE_CCD;
                        }
                        else
                        {
                            currentFlags &= ~PxRigidBodyFlag::eENABLE_CCD;
                        }
                        static_cast<PxRigidDynamic*>(rigidBody.InternalBody)->setRigidBodyFlags(currentFlags);
                    }
                });

            m_Scene->simulate(Time::FixedDeltaTime);
            m_Scene->fetchResults(true);

            // Fetch rigid bodies
            registry.view<TransformComponent, RigidBodyComponent>().each(
                [&registry](entt::entity entity, TransformComponent& transform, RigidBodyComponent& rigidBody) {
                    if (rigidBody.InternalBody == nullptr)
                    {
                        auto name = registry.get<NameComponent>(entity).Name;
                        SNOW_LEOPARD_CORE_WARN("[PhysicsSystem] A rigidBody without shape! Entity name: {0}", name);
                        return;
                    }

                    PxTransform pxTransform = rigidBody.InternalBody->getGlobalPose();
                    PxVec3      pxPosition  = pxTransform.p;
                    PxQuat      pxRotation  = pxTransform.q;
                    transform.Position      = {pxPosition.x, pxPosition.y, pxPosition.z};
                    glm::quat rotation      = {pxRotation.w, pxRotation.x, pxRotation.y, pxRotation.z};
                    transform.SetRotation(rotation);
                });

            // Fetch character controllers
            registry.view<TransformComponent, CharacterControllerComponent>().each(
                [](entt::entity entity, TransformComponent& transform, CharacterControllerComponent& controller) {
                    if (controller.InternalController != nullptr)
                    {
                        auto position      = controller.InternalController->getPosition();
                        transform.Position = {position.x, position.y, position.z};
                    }
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

    /** APIs **/

    /** Character Controller **/
    void
    PhysicsSystem::Move(const CharacterControllerComponent& component, const glm::vec3& movement, float deltaTime) const
    {
        if (component.InternalController != nullptr)
        {
            component.InternalController->move(
                PhysXGLMHelpers::GetPhysXVec3(movement), component.MinMoveDisp, deltaTime, component.Filters);
        }
    }

    /** RigidBody **/
    void PhysicsSystem::AddForce(const RigidBodyComponent& component, const glm::vec3& force) const
    {
        if (component.InternalBody != nullptr && !component.IsStatic)
        {
            PxRigidBody* body = static_cast<PxRigidBody*>(component.InternalBody);
            body->addForce(PhysXGLMHelpers::GetPhysXVec3(force), PxForceMode::eFORCE, true);
        }
    }

    void PhysicsSystem::AddTorque(const RigidBodyComponent& component, const glm::vec3& torque) const
    {
        if (component.InternalBody != nullptr && !component.IsStatic)
        {
            PxRigidBody* body = static_cast<PxRigidBody*>(component.InternalBody);
            body->addTorque(PhysXGLMHelpers::GetPhysXVec3(torque), PxForceMode::eFORCE, true);
        }
    }

    bool PhysicsSystem::SimpleRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance)
    {
        PxRaycastBuffer hit;
        bool            status = m_Scene->raycast(
            PhysXGLMHelpers::GetPhysXVec3(origin), PhysXGLMHelpers::GetPhysXVec3(direction), maxDistance, hit);
        // Because it's a simple API, so ignore most of hit info here.
        if (!status)
        {
            return false;
        }

        return hit.hasAnyHits();
    }

    void PhysicsSystem::OnLogicSceneLoaded(const LogicSceneLoadedEvent& e) { CookPhysicsScene(e.GetLogicScene()); }

    void PhysicsSystem::ReleaseInternalResources()
    {
        if (m_LogicScene != nullptr)
        {
            auto& registry = m_LogicScene->GetRegistry();

            registry.view<RigidBodyComponent>().each([](entt::entity entity, RigidBodyComponent& rigidBody) {
                if (rigidBody.InternalBody != nullptr)
                {
                    rigidBody.InternalBody->release();
                    rigidBody.InternalBody = nullptr;
                }
            });

            registry.view<TerrainColliderComponent>().each(
                [](entt::entity entity, TerrainColliderComponent& terrainCollider) {
                    if (terrainCollider.InternalBody != nullptr)
                    {
                        terrainCollider.InternalBody->release();
                        terrainCollider.InternalBody = nullptr;
                    }
                });

            registry.view<CharacterControllerComponent>().each(
                [](entt::entity entity, CharacterControllerComponent& controller) {
                    if (controller.InternalController != nullptr)
                    {
                        controller.InternalController->release();
                        controller.InternalController = nullptr;
                    }
                });
        }
    }
} // namespace SnowLeopardEngine