#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsErrorCallback.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

namespace SnowLeopardEngine
{
    class PhysicsSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(PhysicsSystem)

        void CookPhysicsScene(const Ref<LogicScene>& logicScene);
        void OnFixedTick();

    private:
        physx::PxDefaultAllocator m_Allocator;
        PhysicsErrorCallback      m_ErrorCallback;
        physx::PxFoundation*      m_Foundation = nullptr;
        physx::PxPhysics*         m_Physics    = nullptr;
        physx::PxScene*           m_Scene      = nullptr;
        Ref<LogicScene>           m_LogicScene = nullptr;
    };
} // namespace SnowLeopardEngine