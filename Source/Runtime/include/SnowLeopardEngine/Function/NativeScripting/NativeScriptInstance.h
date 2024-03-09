#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Engine/LifeTimeComponent.h"

namespace SnowLeopardEngine
{
    class Entity;
    class EngineContext;

    class NativeScriptInstance : public LifeTimeComponent
    {
    public:
        NativeScriptInstance() = default;
        explicit NativeScriptInstance(bool isEditorScript) : m_IsEditorScript(isEditorScript) {}
        virtual ~NativeScriptInstance() = default;

        virtual void OnLoad() override {}

        virtual void OnTick(float deltaTime) override {}

        virtual void OnFixedTick() override {}

        virtual void OnUnload() override {}

        virtual void OnColliderEnter() {}

        bool IsEditorScript() const { return m_IsEditorScript; }

        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool GetEnabled() const { return m_Enabled; }

    protected:
        EngineContext* m_EngineContext  = nullptr;
        Ref<Entity>    m_OwnerEntity    = nullptr;
        bool           m_Enabled        = true;
        bool           m_IsEditorScript = false;

        friend class LogicScene;
    };
} // namespace SnowLeopardEngine
