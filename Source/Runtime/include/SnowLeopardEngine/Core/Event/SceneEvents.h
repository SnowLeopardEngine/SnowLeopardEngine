#pragma once

#include "SnowLeopardEngine/Core/Event/Event.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

namespace SnowLeopardEngine
{
    class LogicScenePreLoadEvent final : public Event
    {
    public:
        DECLARE_EVENT_TYPE(LogicScenePreLoadEvent)

        explicit LogicScenePreLoadEvent(LogicScene* logicScene) : m_LogicScene(logicScene) {}

        LogicScene* GetLogicScene() const { return m_LogicScene; }

        virtual const std::string ToString() const override
        {
            return std::format("LogicScenePreLoadEvent: Preload Scene = {0}", m_LogicScene->GetName());
        }

    private:
        LogicScene* m_LogicScene;
    };

    class LogicSceneLoadedEvent final : public Event
    {
    public:
        DECLARE_EVENT_TYPE(LogicSceneLoadedEvent)

        explicit LogicSceneLoadedEvent(LogicScene* logicScene) : m_LogicScene(logicScene) {}

        LogicScene* GetLogicScene() const { return m_LogicScene; }

        virtual const std::string ToString() const override
        {
            return std::format("LogicSceneLoadingEvent: Loaded Scene = {0}", m_LogicScene->GetName());
        }

    private:
        LogicScene* m_LogicScene;
    };

    class LogicSceneUnloadedEvent final : public Event
    {
    public:
        DECLARE_EVENT_TYPE(LogicSceneUnloadedEvent)

        explicit LogicSceneUnloadedEvent(LogicScene* logicScene) : m_LogicScene(logicScene) {}

        LogicScene* GetLogicScene() const { return m_LogicScene; }

        virtual const std::string ToString() const override
        {
            return std::format("LogicSceneUnloadingEvent: Unloaded Scene = {0}", m_LogicScene->GetName());
        }

    private:
        LogicScene* m_LogicScene;
    };
} // namespace SnowLeopardEngine