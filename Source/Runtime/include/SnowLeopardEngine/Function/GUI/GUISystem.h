#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Event/EventHandler.h"
#include "SnowLeopardEngine/Core/Event/SceneEvents.h"

namespace SnowLeopardEngine
{
    class GUISystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(GUISystem)

        void OnTick(float deltaTime);

    private:
        void OnLogicSceneLoaded(const LogicSceneLoadedEvent& e) { m_Scene = e.GetLogicScene(); }

    private:
        LogicScene* m_Scene = nullptr;

        EventHandler<LogicSceneLoadedEvent> m_LogicSceneLoadedHandler = [this](const LogicSceneLoadedEvent& e) {
            OnLogicSceneLoaded(e);
        };
    };
} // namespace SnowLeopardEngine