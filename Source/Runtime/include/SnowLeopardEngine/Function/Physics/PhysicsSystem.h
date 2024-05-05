#pragma once

#include "PxActor.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Event/Event.h"
#include "SnowLeopardEngine/Core/Event/EventHandler.h"
#include "SnowLeopardEngine/Core/Event/SceneEvents.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Physics/OverlapInfo.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsErrorCallback.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

#include "glm/fwd.hpp"
#include <PxPhysicsAPI.h>
#include <unordered_map>

namespace SnowLeopardEngine
{
    class PhysicsSystem final : public EngineSubSystem, public physx::PxSimulationEventCallback
    {
    public:
        DECLARE_SUBSYSTEM(PhysicsSystem)

        void CookPhysicsScene(LogicScene* logicScene);
        void OnFixedTick();

        void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
        void onWake(physx::PxActor** actors, physx::PxU32 count) override;
        void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
        void onContact(const physx::PxContactPairHeader& pairHeader,
                       const physx::PxContactPair*       pairs,
                       physx::PxU32                      count) override;
        void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
        void onAdvance(const physx::PxRigidBody* const* bodyBuffer,
                       const physx::PxTransform*        poseBuffer,
                       physx::PxU32                     count) override;

        /** APIs **/

        /** Character Controller **/
        void      Move(const CharacterControllerComponent& component, const glm::vec3& movement, float deltaTime) const;
        glm::vec3 GetLinearVelocity(const CharacterControllerComponent& component) const;

        /** RigidBody **/
        void AddForce(const RigidBodyComponent& component, const glm::vec3& force) const;
        void AddTorque(const RigidBodyComponent& component, const glm::vec3& torque) const;

        /** RayCast **/
        bool SimpleRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance);

        /** Overlap **/
        bool OverlapSphere(const glm::vec3& sphereOrigin, float sphereRadius, OverlapInfo& info);

    private:
        void OnLogicSceneLoaded(const LogicSceneLoadedEvent& e);
        void ReleaseInternalResources();

        void OnEntityCreate(const EntityCreateEvent& e);
        void OnEntityDestroy(const EntityDestroyEvent& e);

        physx::PxActor* createSphere(TransformComponent&      transform,
                                     EntityStatusComponent&   entityStatus,
                                     RigidBodyComponent&      rigidBody,
                                     SphereColliderComponent& sphereCollider);

        physx::PxActor* createBox(TransformComponent&    transform,
                                  EntityStatusComponent& entityStatus,
                                  RigidBodyComponent&    rigidBody,
                                  BoxColliderComponent&  boxCollider);

        physx::PxActor* createCapsule(TransformComponent&       transform,
                                      EntityStatusComponent&    entityStatus,
                                      RigidBodyComponent&       rigidBody,
                                      CapsuleColliderComponent& capsuleCollider);

        physx::PxActor* createTerrain(TransformComponent&       transform,
                                      TerrainComponent&         terrain,
                                      TerrainColliderComponent& terrainCollider);

        void createCharacter(TransformComponent& transform, CharacterControllerComponent& characterController);

        physx::PxActor* createMesh(TransformComponent&    transform,
                                   EntityStatusComponent& entityStatus,
                                   RigidBodyComponent&    rigidBody,
                                   MeshFilterComponent&   meshFilter,
                                   MeshColliderComponent& meshCollider);

    private:
        physx::PxDefaultAllocator   m_Allocator;
        PhysicsErrorCallback        m_ErrorCallback;
        physx::PxFoundation*        m_Foundation        = nullptr;
        physx::PxPhysics*           m_Physics           = nullptr;
        physx::PxScene*             m_Scene             = nullptr;
        LogicScene*                 m_LogicScene        = nullptr;
        physx::PxCooking*           m_Cooking           = nullptr;
        physx::PxControllerManager* m_ControllerManager = nullptr;

        std::unordered_map<physx::PxActor*, Entity> m_Actor2EntityMap;

        EventHandler<LogicSceneLoadedEvent> m_LogicSceneLoadedHandler = [this](const LogicSceneLoadedEvent& e) {
            OnLogicSceneLoaded(e);
        };

        EventHandler<EntityCreateEvent> m_EntityCreateHandler = [this](const EntityCreateEvent& e) {
            OnEntityCreate(e);
        };
        EventHandler<EntityDestroyEvent> m_EntityDestroyHandler = [this](const EntityDestroyEvent& e) {
            OnEntityDestroy(e);
        };
    };
} // namespace SnowLeopardEngine