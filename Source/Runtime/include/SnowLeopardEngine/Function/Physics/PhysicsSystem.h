#pragma once

#include <PxPhysicsAPI.h>
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsErrorCallback.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

namespace SnowLeopardEngine
{
    class PhysicsSystem final : public EngineSubSystem , public physx::PxSimulationEventCallback
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
        void onAdvance(const physx::PxRigidBody*const* bodyBuffer, const physx::PxTransform* poseBuffer, physx::PxU32 count) override;

    private:
        physx::PxDefaultAllocator m_Allocator;
        PhysicsErrorCallback      m_ErrorCallback;
        physx::PxFoundation*      m_Foundation = nullptr;
        physx::PxPhysics*         m_Physics    = nullptr;
        physx::PxScene*           m_Scene      = nullptr;
        Ref<LogicScene>           m_LogicScene = nullptr;
    };
} // namespace SnowLeopardEngine