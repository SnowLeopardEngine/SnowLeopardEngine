#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsErrorCallback.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

#include <PxPhysicsAPI.h>

namespace SnowLeopardEngine
{
    class PhysicsSystem final : public EngineSubSystem, public physx::PxSimulationEventCallback
    {
    public:
        DECLARE_SUBSYSTEM(PhysicsSystem)

        void CookPhysicsScene(const Ref<LogicScene>& logicScene);
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
        void Move(const CharacterControllerComponent& component, const glm::vec3& movement, float deltaTime) const;

        /** RigidBody **/
        void AddForce(const RigidBodyComponent& component, const glm::vec3& force) const;
        void AddTorque(const RigidBodyComponent& component, const glm::vec3& torque) const;

    private:
        physx::PxDefaultAllocator   m_Allocator;
        PhysicsErrorCallback        m_ErrorCallback;
        physx::PxFoundation*        m_Foundation = nullptr;
        physx::PxPhysics*           m_Physics    = nullptr;
        physx::PxScene*             m_Scene      = nullptr;
        Ref<LogicScene>             m_LogicScene = nullptr;
        physx::PxControllerManager* m_Controller = nullptr;
    };
} // namespace SnowLeopardEngine