#include "SnowLeopardEngine/Function/Physics/PhysicsSystem.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

// Jolt must be included first: it sets up the configuration macros used by every other Jolt header.
#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/IssueReporting.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

namespace SnowLeopardEngine
{
    namespace
    {
        // ---------------------------------------------------------------------------------------------
        // Limits and layers
        // ---------------------------------------------------------------------------------------------

        constexpr JPH::uint cMaxBodies             = 65536;
        constexpr JPH::uint cNumBodyMutexes        = 0; // 0 = Jolt's default
        constexpr JPH::uint cMaxBodyPairs          = 65536;
        constexpr JPH::uint cMaxContactConstraints = 10240;
        constexpr JPH::uint cTempAllocatorSize     = 32 * 1024 * 1024;

        // Same gravity the PhysX scene used.
        const glm::vec3 cGravity = {0.0f, -9.8f, 0.0f};

        // Guards against degenerate collider sizes, which Jolt asserts on.
        constexpr float cMinExtent = 1.0e-3f;

        namespace Layers
        {
            constexpr JPH::ObjectLayer NonMoving = 0; // static bodies, terrain
            constexpr JPH::ObjectLayer Moving    = 1; // dynamic bodies, character inner bodies
        } // namespace Layers

        namespace BroadPhaseLayers
        {
            constexpr JPH::BroadPhaseLayer NonMoving(0);
            constexpr JPH::BroadPhaseLayer Moving(1);
            constexpr JPH::uint            Count = 2;
        } // namespace BroadPhaseLayers

        class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
        {
        public:
            JPH::uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::Count; }

            JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
            {
                return inLayer == Layers::NonMoving ? BroadPhaseLayers::NonMoving : BroadPhaseLayers::Moving;
            }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
            const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
            {
                return inLayer == BroadPhaseLayers::NonMoving ? "NonMoving" : "Moving";
            }
#endif
        };

        class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
        {
        public:
            bool ShouldCollide(JPH::ObjectLayer inLayer, JPH::BroadPhaseLayer inBroadPhaseLayer) const override
            {
                // Static geometry never needs to be tested against other static geometry.
                return inLayer == Layers::Moving || inBroadPhaseLayer == BroadPhaseLayers::Moving;
            }
        };

        class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
        {
        public:
            bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const override
            {
                return inLayer1 == Layers::Moving || inLayer2 == Layers::Moving;
            }
        };

        // Jolt reports contacts from its worker threads, once per touching sub-shape pair (e.g. per terrain triangle a
        // ball rolls onto). Contacts are counted per body pair so that "enter" fires once when two bodies start
        // touching, like PhysX's eNOTIFY_TOUCH_FOUND did. Enter events are dispatched on the main thread after the step.
        class ContactQueue final : public JPH::ContactListener
        {
        public:
            using Pair = std::pair<PhysicsBodyHandle, PhysicsBodyHandle>;

            void OnContactAdded(const JPH::Body& inBody1,
                                const JPH::Body& inBody2,
                                const JPH::ContactManifold& /*inManifold*/,
                                JPH::ContactSettings& /*ioSettings*/) override
            {
                const Pair                  pair = MakePair(inBody1.GetID(), inBody2.GetID());
                std::lock_guard<std::mutex> lock(m_Mutex);
                if (++m_Touching[pair] == 1)
                {
                    m_Entered.push_back(pair);
                }
            }

            void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
            {
                const Pair                  pair = MakePair(inSubShapePair.GetBody1ID(), inSubShapePair.GetBody2ID());
                std::lock_guard<std::mutex> lock(m_Mutex);
                auto                        it = m_Touching.find(pair);
                if (it != m_Touching.end() && --it->second <= 0)
                {
                    m_Touching.erase(it);
                }
            }

            std::vector<Pair> DrainEntered()
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                std::vector<Pair>           pairs;
                pairs.swap(m_Entered);
                return pairs;
            }

            void Reset()
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_Entered.clear();
                m_Touching.clear();
            }

        private:
            struct PairHash
            {
                size_t operator()(const Pair& pair) const
                {
                    return std::hash<uint64_t> {}((static_cast<uint64_t>(pair.first) << 32) | pair.second);
                }
            };

            static Pair MakePair(const JPH::BodyID& a, const JPH::BodyID& b)
            {
                const PhysicsBodyHandle ha = a.GetIndexAndSequenceNumber();
                const PhysicsBodyHandle hb = b.GetIndexAndSequenceNumber();
                return ha < hb ? Pair {ha, hb} : Pair {hb, ha};
            }

            std::mutex                                   m_Mutex;
            std::vector<Pair>                            m_Entered;
            std::unordered_map<Pair, int, PairHash>      m_Touching;
        };

        // ---------------------------------------------------------------------------------------------
        // Conversions
        // ---------------------------------------------------------------------------------------------

        inline JPH::Vec3  ToJolt(const glm::vec3& v) { return JPH::Vec3(v.x, v.y, v.z); }
        inline JPH::RVec3 ToJoltR(const glm::vec3& v) { return JPH::RVec3(v.x, v.y, v.z); }
        inline JPH::Quat  ToJolt(const glm::quat& q) { return JPH::Quat(q.x, q.y, q.z, q.w); }

        inline glm::vec3 ToGLM(JPH::Vec3Arg v) { return {v.GetX(), v.GetY(), v.GetZ()}; }
#ifdef JPH_DOUBLE_PRECISION
        inline glm::vec3 ToGLM(JPH::DVec3Arg v)
        {
            return {static_cast<float>(v.GetX()), static_cast<float>(v.GetY()), static_cast<float>(v.GetZ())};
        }
#endif
        inline glm::quat ToGLM(JPH::QuatArg q) { return {q.GetW(), q.GetX(), q.GetY(), q.GetZ()}; }

        inline JPH::BodyID ToBodyID(PhysicsBodyHandle handle) { return JPH::BodyID(handle); }

        inline JPH::uint64 ToUserData(Entity entity)
        {
            return static_cast<JPH::uint64>(static_cast<uint32_t>(static_cast<entt::entity>(entity)));
        }

        void JoltTrace(const char* inFormat, ...)
        {
            va_list args;
            va_start(args, inFormat);
            char buffer[1024];
            std::vsnprintf(buffer, sizeof(buffer), inFormat, args);
            va_end(args);
            SNOW_LEOPARD_CORE_INFO("[PhysicsSystem][Jolt] {0}", buffer);
        }

        int GetWorkerThreadCount()
        {
            const int hardwareThreads = static_cast<int>(std::thread::hardware_concurrency());
            return hardwareThreads > 1 ? hardwareThreads - 1 : 1;
        }

        // Colliders are offset in the body's local space, so the body origin stays at the entity's transform.
        JPH::RefConst<JPH::Shape> WithOffset(const JPH::RefConst<JPH::Shape>& shape, const glm::vec3& offset)
        {
            if (offset == glm::vec3(0.0f))
            {
                return shape;
            }
            return new JPH::RotatedTranslatedShape(ToJolt(offset), JPH::Quat::sIdentity(), shape);
        }

        void ApplyMaterial(JPH::BodyCreationSettings& settings, const Ref<PhysicsMaterial>& material)
        {
            // Jolt has a single friction coefficient per body; the dynamic one is the closest match.
            // A missing material means frictionless and non-bouncy, as it did with PhysX.
            settings.mFriction    = material ? material->DynamicFriction : 0.0f;
            settings.mRestitution = material ? material->Bounciness : 0.0f;
        }

        JPH::EMotionQuality ToMotionQuality(bool enableCCD)
        {
            return enableCCD ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete;
        }
    } // namespace

    // -------------------------------------------------------------------------------------------------
    // JoltState
    // -------------------------------------------------------------------------------------------------

    struct PhysicsSystem::JoltState
    {
        JPH::TempAllocatorImpl   TempAllocator {cTempAllocatorSize};
        JPH::JobSystemThreadPool JobSystem {JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, GetWorkerThreadCount()};

        BroadPhaseLayerInterfaceImpl      BroadPhaseLayerInterface;
        ObjectVsBroadPhaseLayerFilterImpl ObjectVsBroadPhaseLayerFilter;
        ObjectLayerPairFilterImpl         ObjectLayerPairFilter;
        ContactQueue                      Contacts;

        JPH::CharacterVsCharacterCollisionSimple CharacterVsCharacter;

        // Declared before Characters so that the characters (and their inner bodies) are destroyed first.
        Scope<JPH::PhysicsSystem> World;

        std::unordered_map<JPH::CharacterVirtual*, JPH::Ref<JPH::CharacterVirtual>> Characters;

        bool HasWorld() const { return World != nullptr; }

        JPH::BodyInterface& Bodies() { return World->GetBodyInterface(); }

        void ResetWorld()
        {
            DestroyWorld();

            World = CreateScope<JPH::PhysicsSystem>();
            World->Init(cMaxBodies,
                        cNumBodyMutexes,
                        cMaxBodyPairs,
                        cMaxContactConstraints,
                        BroadPhaseLayerInterface,
                        ObjectVsBroadPhaseLayerFilter,
                        ObjectLayerPairFilter);
            World->SetGravity(ToJolt(cGravity));
            World->SetContactListener(&Contacts);
        }

        void DestroyWorld()
        {
            for (auto& [character, ref] : Characters)
            {
                CharacterVsCharacter.Remove(character);
            }
            Characters.clear();
            World.reset();
            Contacts.Reset();
        }

        ~JoltState() { DestroyWorld(); }
    };

    // -------------------------------------------------------------------------------------------------
    // Lifetime
    // -------------------------------------------------------------------------------------------------

    PhysicsSystem::PhysicsSystem()
    {
        JPH::RegisterDefaultAllocator();
        JPH::Trace              = &JoltTrace;
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        m_Jolt = CreateScope<JoltState>();

        Subscribe(m_LogicSceneLoadedHandler);
        Subscribe(m_EntityCreateHandler);
        Subscribe(m_EntityDestroyHandler);

        SNOW_LEOPARD_CORE_INFO("[PhysicsSystem] Initialized (Jolt Physics {0}.{1}.{2})",
                               JPH_VERSION_MAJOR,
                               JPH_VERSION_MINOR,
                               JPH_VERSION_PATCH);
        m_State = SystemState::InitOk;
    }

    PhysicsSystem::~PhysicsSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[PhysicsSystem] Shutting Down...");

        Unsubscribe(m_EntityDestroyHandler);
        Unsubscribe(m_EntityCreateHandler);
        Unsubscribe(m_LogicSceneLoadedHandler);

        // Destroying the world releases every body and character. The scene's components are not touched here
        // because the scene may already be gone at shutdown.
        m_Body2EntityMap.clear();
        m_Jolt.reset();

        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;

        m_State = SystemState::ShutdownOk;
    }

    // -------------------------------------------------------------------------------------------------
    // Scene
    // -------------------------------------------------------------------------------------------------

    void PhysicsSystem::CookPhysicsScene(LogicScene* logicScene)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        // Re-cooking the same scene: clear the handles its components still hold. A previous, different scene is
        // left alone (it may already be destroyed); resetting the world below drops all of its bodies anyway.
        if (logicScene == m_LogicScene)
        {
            ReleaseInternalResources();
        }

        m_LogicScene = logicScene;
        m_Body2EntityMap.clear();
        m_Jolt->ResetWorld();

        // Cook the scene from logic
        auto& registry = logicScene->GetRegistry();

        // Case 1: RigidBody + SphereCollider
        registry.view<TransformComponent, EntityStatusComponent, RigidBodyComponent, SphereColliderComponent>().each(
            [this, logicScene](entt::entity             entity,
                               TransformComponent&      transform,
                               EntityStatusComponent&   entityStatus,
                               RigidBodyComponent&      rigidBody,
                               SphereColliderComponent& sphereCollider) {
                createSphere({entity, logicScene}, transform, entityStatus, rigidBody, sphereCollider);
            });

        // Case 2: RigidBody + BoxCollider
        registry.view<TransformComponent, EntityStatusComponent, RigidBodyComponent, BoxColliderComponent>().each(
            [this, logicScene](entt::entity           entity,
                               TransformComponent&    transform,
                               EntityStatusComponent& entityStatus,
                               RigidBodyComponent&    rigidBody,
                               BoxColliderComponent&  boxCollider) {
                createBox({entity, logicScene}, transform, entityStatus, rigidBody, boxCollider);
            });

        // case3: RigidBodyComponent + CapsuleColliderComponent
        registry.view<TransformComponent, EntityStatusComponent, RigidBodyComponent, CapsuleColliderComponent>().each(
            [this, logicScene](entt::entity              entity,
                               TransformComponent&       transform,
                               EntityStatusComponent&    entityStatus,
                               RigidBodyComponent&       rigidBody,
                               CapsuleColliderComponent& capsuleCollider) {
                createCapsule({entity, logicScene}, transform, entityStatus, rigidBody, capsuleCollider);
            });

        // case4: TerrainComponent + TerrainColliderComponent
        registry.view<TransformComponent, TerrainComponent, TerrainColliderComponent>().each(
            [this, logicScene](entt::entity              entity,
                               TransformComponent&       transform,
                               TerrainComponent&         terrain,
                               TerrainColliderComponent& terrainCollider) {
                createTerrain({entity, logicScene}, transform, terrain, terrainCollider);
            });

        // case5: CharacterController
        registry.view<TransformComponent, CharacterControllerComponent>().each(
            [this, logicScene](
                entt::entity entity, TransformComponent& transform, CharacterControllerComponent& characterController) {
                createCharacter({entity, logicScene}, transform, characterController);
            });

        // case6: MeshFilterComponent + MeshColliderComponent + RigidBodyComponent
        registry
            .view<TransformComponent,
                  EntityStatusComponent,
                  RigidBodyComponent,
                  MeshFilterComponent,
                  MeshColliderComponent>()
            .each([this, logicScene](entt::entity           entity,
                                     TransformComponent&    transform,
                                     EntityStatusComponent& entityStatus,
                                     RigidBodyComponent&    rigidBody,
                                     MeshFilterComponent&   meshFilter,
                                     MeshColliderComponent& meshCollider) {
                createMesh({entity, logicScene}, transform, entityStatus, rigidBody, meshFilter, meshCollider);
            });

        // TODO: More cases

        // Everything was just inserted in one go; rebuild the broad phase tree once instead of incrementally.
        m_Jolt->World->OptimizeBroadPhase();
    }

    void PhysicsSystem::OnFixedTick()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        if (m_LogicScene == nullptr || !m_Jolt->HasWorld())
        {
            return;
        }

        auto& registry = m_LogicScene->GetRegistry();
        auto& bodies   = m_Jolt->Bodies();

        // Push edited rigid body properties into the simulation
        registry.view<TransformComponent, EntityStatusComponent, RigidBodyComponent>().each(
            [this, &registry, &bodies](entt::entity           entity,
                                       TransformComponent&    transform,
                                       EntityStatusComponent& entityStatus,
                                       RigidBodyComponent&    rigidBody) {
                rigidBody.IsStatic = entityStatus.IsStatic;
                if (rigidBody.InternalBody == InvalidPhysicsBody)
                {
                    auto name = registry.get<NameComponent>(entity).Name;
                    SNOW_LEOPARD_CORE_WARN("[PhysicsSystem] A rigidBody without shape! Entity name: {0}", name);
                    return;
                }

                if (rigidBody.IsStatic)
                {
                    return;
                }

                const JPH::BodyID   bodyID         = ToBodyID(rigidBody.InternalBody);
                JPH::EMotionQuality currentQuality = JPH::EMotionQuality::Discrete;
                {
                    JPH::BodyLockWrite lock(m_Jolt->World->GetBodyLockInterface(), bodyID);
                    if (!lock.Succeeded() || !lock.GetBody().IsDynamic())
                    {
                        return;
                    }

                    JPH::Body&             body   = lock.GetBody();
                    JPH::MotionProperties* motion = body.GetMotionProperties();
                    motion->SetLinearDamping(rigidBody.LinearDamping);
                    motion->SetAngularDamping(rigidBody.AngularDamping);

                    const float mass = std::max(rigidBody.Mass, cMinExtent);
                    if (std::abs(motion->GetInverseMass() - 1.0f / mass) > 1.0e-6f)
                    {
                        JPH::MassProperties massProperties = body.GetShape()->GetMassProperties();
                        massProperties.ScaleToMass(mass);
                        motion->SetMassProperties(JPH::EAllowedDOFs::All, massProperties);
                    }

                    currentQuality = motion->GetMotionQuality();
                }

                // SetMotionQuality takes its own body lock, so it has to run after the scope above.
                const JPH::EMotionQuality wantedQuality = ToMotionQuality(rigidBody.EnableCCD);
                if (currentQuality != wantedQuality)
                {
                    bodies.SetMotionQuality(bodyID, wantedQuality);
                }
            });

        m_Jolt->World->Update(Time::FixedDeltaTime, 1, &m_Jolt->TempAllocator, &m_Jolt->JobSystem);

        // Fetch rigid bodies
        registry.view<TransformComponent, RigidBodyComponent>().each(
            [&bodies](entt::entity entity, TransformComponent& transform, RigidBodyComponent& rigidBody) {
                if (rigidBody.InternalBody == InvalidPhysicsBody || rigidBody.IsStatic)
                {
                    return;
                }

                JPH::RVec3 position;
                JPH::Quat  rotation;
                bodies.GetPositionAndRotation(ToBodyID(rigidBody.InternalBody), position, rotation);
                transform.Position = ToGLM(position);
                transform.SetRotation(ToGLM(rotation));
            });

        // Fetch character controllers (the position is the capsule center, as it was with the PhysX controller)
        registry.view<TransformComponent, CharacterControllerComponent>().each(
            [](entt::entity entity, TransformComponent& transform, CharacterControllerComponent& controller) {
                if (controller.InternalController != nullptr)
                {
                    transform.Position = ToGLM(controller.InternalController->GetPosition());
                }
            });

        DispatchContactEvents();
    }

    void PhysicsSystem::DispatchContactEvents()
    {
        auto pairs = m_Jolt->Contacts.DrainEntered();
        if (pairs.empty() || m_LogicScene == nullptr)
        {
            return;
        }

        auto& registry = m_LogicScene->GetRegistry();

        auto notify = [this, &registry](PhysicsBodyHandle body) {
            auto it = m_Body2EntityMap.find(body);
            if (it == m_Body2EntityMap.end())
            {
                return;
            }

            const entt::entity entity = it->second;
            if (!registry.valid(entity))
            {
                return;
            }

            if (auto* scripting = registry.try_get<NativeScriptingComponent>(entity);
                scripting != nullptr && scripting->ScriptInstance != nullptr)
            {
                scripting->ScriptInstance->OnColliderEnter();
            }
        };

        for (const auto& [body1, body2] : pairs)
        {
            notify(body1);
            notify(body2);
        }
    }

    /** APIs **/

    /** Character Controller **/
    void
    PhysicsSystem::Move(const CharacterControllerComponent& component, const glm::vec3& movement, float deltaTime) const
    {
        JPH::CharacterVirtual* character = component.InternalController;
        if (character == nullptr || deltaTime <= 0.0f || !m_Jolt->HasWorld())
        {
            return;
        }

        // The PhysX controller took a displacement; Jolt's virtual character takes a velocity.
        character->SetLinearVelocity(ToJolt(movement) / deltaTime);

        JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;
        // Callers apply their own gravity through the displacement, like they did with the PhysX controller.
        updateSettings.mStickToFloorStepDown = JPH::Vec3::sZero();
        updateSettings.mWalkStairsStepUp     = JPH::Vec3(0.0f, component.StepOffset, 0.0f);

        // The character must not collide with its own inner body.
        JPH::IgnoreSingleBodyFilter ignoreSelf(character->GetInnerBodyID());

        character->ExtendedUpdate(deltaTime,
                                  m_Jolt->World->GetGravity(),
                                  updateSettings,
                                  m_Jolt->World->GetDefaultBroadPhaseLayerFilter(Layers::Moving),
                                  m_Jolt->World->GetDefaultLayerFilter(Layers::Moving),
                                  ignoreSelf,
                                  {},
                                  m_Jolt->TempAllocator);
    }

    glm::vec3 PhysicsSystem::GetLinearVelocity(const CharacterControllerComponent& component) const
    {
        if (component.InternalController != nullptr)
        {
            return ToGLM(component.InternalController->GetLinearVelocity());
        }

        return glm::vec3(0);
    }

    /** RigidBody **/
    void PhysicsSystem::AddForce(const RigidBodyComponent& component, const glm::vec3& force) const
    {
        if (component.InternalBody != InvalidPhysicsBody && !component.IsStatic && m_Jolt->HasWorld())
        {
            m_Jolt->Bodies().AddForce(ToBodyID(component.InternalBody), ToJolt(force));
        }
    }

    void PhysicsSystem::AddTorque(const RigidBodyComponent& component, const glm::vec3& torque) const
    {
        if (component.InternalBody != InvalidPhysicsBody && !component.IsStatic && m_Jolt->HasWorld())
        {
            m_Jolt->Bodies().AddTorque(ToBodyID(component.InternalBody), ToJolt(torque));
        }
    }

    bool PhysicsSystem::SimpleRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance)
    {
        if (!m_Jolt->HasWorld() || maxDistance <= 0.0f || glm::length(direction) == 0.0f)
        {
            return false;
        }

        // Jolt encodes the ray length in the direction vector.
        const JPH::RRayCast ray {ToJoltR(origin), ToJolt(glm::normalize(direction) * maxDistance)};

        // Because it's a simple API, so ignore most of hit info here.
        JPH::RayCastResult hit;
        return m_Jolt->World->GetNarrowPhaseQuery().CastRay(ray, hit);
    }

    bool PhysicsSystem::OverlapSphere(const glm::vec3& sphereOrigin, float sphereRadius, OverlapInfo& info)
    {
        if (!m_Jolt->HasWorld() || sphereRadius <= 0.0f)
        {
            return false;
        }

        JPH::SphereShape sphere(sphereRadius);
        sphere.SetEmbedded(); // stack-allocated shape: keep the ref count from deleting it

        JPH::AllHitCollisionCollector<JPH::CollideShapeCollector> collector;
        m_Jolt->World->GetNarrowPhaseQuery().CollideShape(&sphere,
                                                          JPH::Vec3::sOne(),
                                                          JPH::RMat44::sTranslation(ToJoltR(sphereOrigin)),
                                                          JPH::CollideShapeSettings(),
                                                          JPH::RVec3::sZero(),
                                                          collector);

        // One body can produce several hits (e.g. a compound or a heightfield); report each entity once.
        std::unordered_set<PhysicsBodyHandle> seen;
        for (const auto& hit : collector.mHits)
        {
            const PhysicsBodyHandle body = hit.mBodyID2.GetIndexAndSequenceNumber();
            if (!seen.insert(body).second)
            {
                continue;
            }

            if (auto it = m_Body2EntityMap.find(body); it != m_Body2EntityMap.end())
            {
                info.OverlappedEntities.emplace_back(it->second);
            }
        }

        return !collector.mHits.empty();
    }

    void PhysicsSystem::OnLogicSceneLoaded(const LogicSceneLoadedEvent& e) { CookPhysicsScene(e.GetLogicScene()); }

    void PhysicsSystem::ReleaseInternalResources()
    {
        if (m_LogicScene != nullptr)
        {
            auto& registry = m_LogicScene->GetRegistry();

            registry.view<RigidBodyComponent>().each(
                [this](entt::entity entity, RigidBodyComponent& rigidBody) { releaseBody(rigidBody.InternalBody); });

            registry.view<TerrainColliderComponent>().each(
                [this](entt::entity entity, TerrainColliderComponent& terrainCollider) {
                    releaseBody(terrainCollider.InternalBody);
                });

            registry.view<CharacterControllerComponent>().each(
                [this](entt::entity entity, CharacterControllerComponent& controller) {
                    releaseCharacter(controller.InternalController);
                });
        }

        m_Body2EntityMap.clear();
    }

    void PhysicsSystem::OnEntityCreate(const EntityCreateEvent& e)
    {
        if (!m_Jolt->HasWorld())
        {
            return;
        }

        auto entity = e.GetEntity();

        TransformComponent&    transformComp    = entity.GetComponent<TransformComponent>();
        EntityStatusComponent& entityStatusComp = entity.GetComponent<EntityStatusComponent>();

        if (entity.HasComponent<SphereColliderComponent>() && entity.HasComponent<RigidBodyComponent>())
        {
            RigidBodyComponent&      rigidBodyComp      = entity.GetComponent<RigidBodyComponent>();
            SphereColliderComponent& sphereColliderComp = entity.GetComponent<SphereColliderComponent>();
            createSphere(entity, transformComp, entityStatusComp, rigidBodyComp, sphereColliderComp);
        }
        if (entity.HasComponent<BoxColliderComponent>() && entity.HasComponent<RigidBodyComponent>())
        {
            BoxColliderComponent& boxColliderComp = entity.GetComponent<BoxColliderComponent>();
            RigidBodyComponent&   rigidBodyComp   = entity.GetComponent<RigidBodyComponent>();
            createBox(entity, transformComp, entityStatusComp, rigidBodyComp, boxColliderComp);
        }
        if (entity.HasComponent<CapsuleColliderComponent>() && entity.HasComponent<RigidBodyComponent>())
        {
            CapsuleColliderComponent& capsuleColliderComp = entity.GetComponent<CapsuleColliderComponent>();
            RigidBodyComponent&       rigidBodyComp       = entity.GetComponent<RigidBodyComponent>();
            createCapsule(entity, transformComp, entityStatusComp, rigidBodyComp, capsuleColliderComp);
        }
        if (entity.HasComponent<TerrainComponent>() && entity.HasComponent<TerrainColliderComponent>())
        {
            TerrainComponent&         terrainComp         = entity.GetComponent<TerrainComponent>();
            TerrainColliderComponent& terrainColliderComp = entity.GetComponent<TerrainColliderComponent>();
            createTerrain(entity, transformComp, terrainComp, terrainColliderComp);
        }
        if (entity.HasComponent<CharacterControllerComponent>())
        {
            CharacterControllerComponent& characterControllerComp = entity.GetComponent<CharacterControllerComponent>();
            createCharacter(entity, transformComp, characterControllerComp);
        }
        if (entity.HasComponent<MeshColliderComponent>() && entity.HasComponent<MeshFilterComponent>() &&
            entity.HasComponent<RigidBodyComponent>())
        {
            MeshFilterComponent&   meshFilterComponent   = entity.GetComponent<MeshFilterComponent>();
            MeshColliderComponent& meshColliderComponent = entity.GetComponent<MeshColliderComponent>();
            RigidBodyComponent&    rigidBodyComp         = entity.GetComponent<RigidBodyComponent>();
            createMesh(entity, transformComp, entityStatusComp, rigidBodyComp, meshFilterComponent, meshColliderComponent);
        }
    }

    void PhysicsSystem::OnEntityDestroy(const EntityDestroyEvent& e)
    {
        auto entity = e.GetEntity();
        if (entity.HasComponent<RigidBodyComponent>())
        {
            releaseBody(entity.GetComponent<RigidBodyComponent>().InternalBody);
        }
        if (entity.HasComponent<TerrainColliderComponent>())
        {
            releaseBody(entity.GetComponent<TerrainColliderComponent>().InternalBody);
        }
        if (entity.HasComponent<CharacterControllerComponent>())
        {
            releaseCharacter(entity.GetComponent<CharacterControllerComponent>().InternalController);
        }
    }

    // -------------------------------------------------------------------------------------------------
    // Body creation
    // -------------------------------------------------------------------------------------------------

    namespace
    {
        // Creates a body from a finished shape; shared by every collider type.
        PhysicsBodyHandle CreateBody(JPH::BodyInterface&              bodies,
                                     Entity                           entity,
                                     const JPH::RefConst<JPH::Shape>& shape,
                                     const TransformComponent&        transform,
                                     bool                             isStatic,
                                     const RigidBodyComponent*        rigidBody,
                                     const Ref<PhysicsMaterial>&      material,
                                     bool                             isTrigger,
                                     bool                             applyRotation = true)
        {
            JPH::BodyCreationSettings settings(shape,
                                               ToJoltR(transform.Position),
                                               applyRotation ? ToJolt(transform.GetRotation()) :
                                                               JPH::Quat::sIdentity(),
                                               isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
                                               isStatic ? Layers::NonMoving : Layers::Moving);
            settings.mUserData = ToUserData(entity);
            settings.mIsSensor = isTrigger;
            ApplyMaterial(settings, material);

            if (!isStatic && rigidBody != nullptr)
            {
                settings.mOverrideMassProperties       = JPH::EOverrideMassProperties::CalculateInertia;
                settings.mMassPropertiesOverride.mMass = std::max(rigidBody->Mass, cMinExtent);
                settings.mLinearDamping                = rigidBody->LinearDamping;
                settings.mAngularDamping               = rigidBody->AngularDamping;
                settings.mMotionQuality                = ToMotionQuality(rigidBody->EnableCCD);
            }

            const JPH::BodyID id =
                bodies.CreateAndAddBody(settings, isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate);
            if (id.IsInvalid())
            {
                SNOW_LEOPARD_CORE_ERROR("[PhysicsSystem] Out of bodies (limit {0}) while creating a collider", cMaxBodies);
                return InvalidPhysicsBody;
            }

            return id.GetIndexAndSequenceNumber();
        }
    } // namespace

    PhysicsBodyHandle PhysicsSystem::createSphere(Entity                   entity,
                                                  TransformComponent&      transform,
                                                  EntityStatusComponent&   entityStatus,
                                                  RigidBodyComponent&      rigidBody,
                                                  SphereColliderComponent& sphereCollider)
    {
        rigidBody.IsStatic = entityStatus.IsStatic;

        if (sphereCollider.Radius == 0)
        {
            sphereCollider.Radius = 0.5f * transform.Scale.x;
        }

        JPH::RefConst<JPH::Shape> shape = new JPH::SphereShape(std::max(sphereCollider.Radius, cMinExtent));

        rigidBody.InternalBody = CreateBody(m_Jolt->Bodies(),
                                            entity,
                                            shape,
                                            transform,
                                            entityStatus.IsStatic,
                                            &rigidBody,
                                            sphereCollider.Material,
                                            sphereCollider.IsTrigger);
        if (rigidBody.InternalBody != InvalidPhysicsBody)
        {
            m_Body2EntityMap[rigidBody.InternalBody] = entity;
        }
        return rigidBody.InternalBody;
    }

    PhysicsBodyHandle PhysicsSystem::createBox(Entity                 entity,
                                               TransformComponent&    transform,
                                               EntityStatusComponent& entityStatus,
                                               RigidBodyComponent&    rigidBody,
                                               BoxColliderComponent&  boxCollider)
    {
        rigidBody.IsStatic = entityStatus.IsStatic;

        if (boxCollider.Size == glm::vec3(0, 0, 0))
        {
            boxCollider.Size = transform.Scale;
        }

        const glm::vec3 halfExtent = glm::max(boxCollider.Size * 0.5f, glm::vec3(cMinExtent));
        const float     convexRadius =
            std::min({JPH::cDefaultConvexRadius, halfExtent.x, halfExtent.y, halfExtent.z});

        JPH::RefConst<JPH::Shape> shape =
            WithOffset(new JPH::BoxShape(ToJolt(halfExtent), convexRadius), boxCollider.Offset);

        rigidBody.InternalBody = CreateBody(m_Jolt->Bodies(),
                                            entity,
                                            shape,
                                            transform,
                                            entityStatus.IsStatic,
                                            &rigidBody,
                                            boxCollider.Material,
                                            boxCollider.IsTrigger);
        if (rigidBody.InternalBody != InvalidPhysicsBody)
        {
            m_Body2EntityMap[rigidBody.InternalBody] = entity;
        }
        return rigidBody.InternalBody;
    }

    PhysicsBodyHandle PhysicsSystem::createCapsule(Entity                    entity,
                                                   TransformComponent&       transform,
                                                   EntityStatusComponent&    entityStatus,
                                                   RigidBodyComponent&       rigidBody,
                                                   CapsuleColliderComponent& capsuleCollider)
    {
        rigidBody.IsStatic = entityStatus.IsStatic;

        // Height is the cylinder part (distance between the two hemisphere centers); the capsule is Y-up.
        const float               radius     = std::max(capsuleCollider.Radius, cMinExtent);
        const float               halfHeight = capsuleCollider.Height * 0.5f;
        JPH::RefConst<JPH::Shape> core       = halfHeight > cMinExtent ?
                                                   static_cast<JPH::Shape*>(new JPH::CapsuleShape(halfHeight, radius)) :
                                                   static_cast<JPH::Shape*>(new JPH::SphereShape(radius));

        rigidBody.InternalBody = CreateBody(m_Jolt->Bodies(),
                                            entity,
                                            WithOffset(core, capsuleCollider.Offset),
                                            transform,
                                            entityStatus.IsStatic,
                                            &rigidBody,
                                            capsuleCollider.Material,
                                            capsuleCollider.IsTrigger);
        if (rigidBody.InternalBody != InvalidPhysicsBody)
        {
            m_Body2EntityMap[rigidBody.InternalBody] = entity;
        }
        return rigidBody.InternalBody;
    }

    PhysicsBodyHandle PhysicsSystem::createTerrain(Entity                    entity,
                                                   TransformComponent&       transform,
                                                   TerrainComponent&         terrain,
                                                   TerrainColliderComponent& terrainCollider)
    {
        const HeightMap& heightMap = terrain.TerrainHeightMap;

        // Jolt height fields are square. The terrain mesh assumes a square map as well.
        const uint32_t sampleCount = std::min(heightMap.Width, heightMap.Height);
        if (heightMap.Width != heightMap.Height)
        {
            SNOW_LEOPARD_CORE_WARN("[PhysicsSystem] Terrain height map is {0}x{1}; only the {2}x{2} corner collides",
                                   heightMap.Width,
                                   heightMap.Height,
                                   sampleCount);
        }
        if (sampleCount < 4)
        {
            SNOW_LEOPARD_CORE_ERROR("[PhysicsSystem] Terrain height map is too small for a collider ({0} samples)",
                                    sampleCount);
            return InvalidPhysicsBody;
        }

        // Same layout as HeightfieldMesh: vertex (column, row) sits at (column * XScale, height * YScale, row * ZScale).
        std::vector<float> samples(static_cast<size_t>(sampleCount) * sampleCount);
        for (uint32_t row = 0; row < sampleCount; ++row)
        {
            for (uint32_t column = 0; column < sampleCount; ++column)
            {
                samples[static_cast<size_t>(row) * sampleCount + column] = heightMap.Get(row, column);
            }
        }

        JPH::HeightFieldShapeSettings shapeSettings(samples.data(),
                                                    JPH::Vec3::sZero(),
                                                    JPH::Vec3(terrain.XScale, terrain.YScale, terrain.ZScale),
                                                    sampleCount);
        JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
        if (shapeResult.HasError())
        {
            SNOW_LEOPARD_CORE_ERROR("[PhysicsSystem] Failed to create the terrain collider: {0}",
                                    shapeResult.GetError().c_str());
            return InvalidPhysicsBody;
        }

        // Terrain only follows the entity's position, as before.
        terrainCollider.InternalBody = CreateBody(m_Jolt->Bodies(),
                                                  entity,
                                                  shapeResult.Get(),
                                                  transform,
                                                  true,
                                                  nullptr,
                                                  terrainCollider.Material,
                                                  terrainCollider.IsTrigger,
                                                  false);
        if (terrainCollider.InternalBody != InvalidPhysicsBody)
        {
            m_Body2EntityMap[terrainCollider.InternalBody] = entity;
        }
        return terrainCollider.InternalBody;
    }

    void PhysicsSystem::createCharacter(Entity                        entity,
                                        TransformComponent&           transform,
                                        CharacterControllerComponent& characterController)
    {
        // As with the PhysX capsule controller, Height is the distance between the hemisphere centers and the
        // character position is the capsule center.
        const float radius     = std::max(characterController.Radius, cMinExtent);
        const float halfHeight = std::max(characterController.Height * 0.5f, cMinExtent);

        JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
        settings->mShape                                 = new JPH::CapsuleShape(halfHeight, radius);
        settings->mUp                                    = JPH::Vec3::sAxisY();
        settings->mMaxSlopeAngle                         = JPH::DegreesToRadians(characterController.SlopeLimit);
        // Only contacts with the lower hemisphere can support the character.
        settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), halfHeight);
        // An inner kinematic body keeps the character visible to ray casts, overlaps and other bodies, like the
        // kinematic actor behind a PhysX controller.
        settings->mInnerBodyShape = settings->mShape;
        settings->mInnerBodyLayer = Layers::Moving;

        const glm::vec3 position = transform.Position + characterController.Offset;

        JPH::Ref<JPH::CharacterVirtual> character = new JPH::CharacterVirtual(
            settings, ToJoltR(position), JPH::Quat::sIdentity(), ToUserData(entity), m_Jolt->World.get());

        character->SetCharacterVsCharacterCollision(&m_Jolt->CharacterVsCharacter);
        m_Jolt->CharacterVsCharacter.Add(character);
        m_Jolt->Characters[character.GetPtr()] = character;

        if (!character->GetInnerBodyID().IsInvalid())
        {
            m_Body2EntityMap[character->GetInnerBodyID().GetIndexAndSequenceNumber()] = entity;
        }

        characterController.InternalController = character.GetPtr();
    }

    PhysicsBodyHandle PhysicsSystem::createMesh(Entity                 entity,
                                                TransformComponent&    transform,
                                                EntityStatusComponent& entityStatus,
                                                RigidBodyComponent&    rigidBody,
                                                MeshFilterComponent&   meshFilter,
                                                MeshColliderComponent& meshCollider)
    {
        rigidBody.IsStatic = entityStatus.IsStatic;

        if (meshFilter.Meshes == nullptr)
        {
            SNOW_LEOPARD_CORE_WARN("[PhysicsSystem] MeshCollider without a mesh, skipped");
            return InvalidPhysicsBody;
        }

        // Like the PhysX version, the collider is the convex hull of every vertex of the model.
        JPH::Array<JPH::Vec3> points;
        for (const auto& meshItem : meshFilter.Meshes->Items)
        {
            for (const auto& vertex : meshItem.Data.Vertices)
            {
                points.push_back(ToJolt(vertex.Position));
            }
        }

        JPH::ConvexHullShapeSettings    shapeSettings(points);
        JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
        if (shapeResult.HasError())
        {
            SNOW_LEOPARD_CORE_ERROR("[PhysicsSystem] Failed to build the convex hull for a MeshCollider: {0}",
                                    shapeResult.GetError().c_str());
            return InvalidPhysicsBody;
        }

        rigidBody.InternalBody = CreateBody(m_Jolt->Bodies(),
                                            entity,
                                            WithOffset(shapeResult.Get(), meshCollider.Offset),
                                            transform,
                                            entityStatus.IsStatic,
                                            &rigidBody,
                                            meshCollider.Material,
                                            meshCollider.IsTrigger);
        if (rigidBody.InternalBody != InvalidPhysicsBody)
        {
            m_Body2EntityMap[rigidBody.InternalBody] = entity;
        }
        return rigidBody.InternalBody;
    }

    // -------------------------------------------------------------------------------------------------
    // Release
    // -------------------------------------------------------------------------------------------------

    void PhysicsSystem::releaseBody(PhysicsBodyHandle& body)
    {
        if (body == InvalidPhysicsBody)
        {
            return;
        }

        m_Body2EntityMap.erase(body);
        if (m_Jolt->HasWorld())
        {
            const JPH::BodyID id = ToBodyID(body);
            auto&             bodies = m_Jolt->Bodies();
            if (bodies.IsAdded(id))
            {
                bodies.RemoveBody(id);
            }
            bodies.DestroyBody(id);
        }
        body = InvalidPhysicsBody;
    }

    void PhysicsSystem::releaseCharacter(PhysicsCharacterHandle& character)
    {
        if (character == nullptr)
        {
            return;
        }

        if (!character->GetInnerBodyID().IsInvalid())
        {
            m_Body2EntityMap.erase(character->GetInnerBodyID().GetIndexAndSequenceNumber());
        }

        m_Jolt->CharacterVsCharacter.Remove(character);
        m_Jolt->Characters.erase(character); // drops the last reference; the inner body goes with it
        character = nullptr;
    }
} // namespace SnowLeopardEngine
