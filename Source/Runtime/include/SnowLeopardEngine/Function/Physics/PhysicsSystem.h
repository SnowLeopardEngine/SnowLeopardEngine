#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Event/Event.h"
#include "SnowLeopardEngine/Core/Event/EventHandler.h"
#include "SnowLeopardEngine/Core/Event/SceneEvents.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Physics/OverlapInfo.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsTypes.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

#include <unordered_map>

namespace SnowLeopardEngine
{
    // Physics backed by Jolt Physics. Jolt types never leave PhysicsSystem.cpp: components hold
    // opaque handles (PhysicsTypes.h) and everything else lives in the private JoltState.
    class PhysicsSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(PhysicsSystem)

        void CookPhysicsScene(LogicScene* logicScene);
        void OnFixedTick();

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
        struct JoltState;

        void OnLogicSceneLoaded(const LogicSceneLoadedEvent& e);
        void ReleaseInternalResources();
        void DispatchContactEvents();

        void OnEntityCreate(const EntityCreateEvent& e);
        void OnEntityDestroy(const EntityDestroyEvent& e);

        PhysicsBodyHandle createSphere(Entity                   entity,
                                       TransformComponent&      transform,
                                       EntityStatusComponent&   entityStatus,
                                       RigidBodyComponent&      rigidBody,
                                       SphereColliderComponent& sphereCollider);

        PhysicsBodyHandle createBox(Entity                 entity,
                                    TransformComponent&    transform,
                                    EntityStatusComponent& entityStatus,
                                    RigidBodyComponent&    rigidBody,
                                    BoxColliderComponent&  boxCollider);

        PhysicsBodyHandle createCapsule(Entity                    entity,
                                        TransformComponent&       transform,
                                        EntityStatusComponent&    entityStatus,
                                        RigidBodyComponent&       rigidBody,
                                        CapsuleColliderComponent& capsuleCollider);

        PhysicsBodyHandle createTerrain(Entity                    entity,
                                        TransformComponent&       transform,
                                        TerrainComponent&         terrain,
                                        TerrainColliderComponent& terrainCollider);

        void createCharacter(Entity                        entity,
                             TransformComponent&           transform,
                             CharacterControllerComponent& characterController);

        PhysicsBodyHandle createMesh(Entity                 entity,
                                     TransformComponent&    transform,
                                     EntityStatusComponent& entityStatus,
                                     RigidBodyComponent&    rigidBody,
                                     MeshFilterComponent&   meshFilter,
                                     MeshColliderComponent& meshCollider);

        void releaseBody(PhysicsBodyHandle& body);
        void releaseCharacter(PhysicsCharacterHandle& character);

    private:
        Scope<JoltState> m_Jolt;
        LogicScene*      m_LogicScene = nullptr;

        std::unordered_map<PhysicsBodyHandle, Entity> m_Body2EntityMap;

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
