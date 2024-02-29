#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Engine/LifeTimeComponent.h"

namespace SnowLeopardEngine
{
    class Entity;

    class NativeScriptInstance : public LifeTimeComponent
    {
    public:
        virtual void OnLoad() override {}

        virtual void OnTick(float deltaTime) override {}

        virtual void OnFixedTick() override {}

        virtual void OnUnload() override {}

        virtual void OnColliderEnter() {}

        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool GetEnabled() const { return m_Enabled; }

    protected:
        Ref<Entity> m_OwnerEntity = nullptr;
        bool        m_Enabled     = true;
        friend class LogicScene;
    };
} // namespace SnowLeopardEngine
