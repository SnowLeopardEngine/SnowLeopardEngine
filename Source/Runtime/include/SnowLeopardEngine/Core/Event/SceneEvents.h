#pragma once

#include "SnowLeopardEngine/Core/Event/Event.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"
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
            return fmt::format("LogicScenePreLoadEvent: Preload Scene = {0}", m_LogicScene->GetName());
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
            return fmt::format("LogicSceneLoadingEvent: Loaded Scene = {0}", m_LogicScene->GetName());
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
            return fmt::format("LogicSceneUnloadingEvent: Unloaded Scene = {0}", m_LogicScene->GetName());
        }

    private:
        LogicScene* m_LogicScene;
    };

    class EntityCreateEvent final : public Event
    {
    public:
        DECLARE_EVENT_TYPE(EntityCreateEvent)

        explicit EntityCreateEvent(Entity& entity) : m_Entity(entity) {}

        Entity GetEntity() const { return m_Entity; }

        virtual const std::string ToString() const override
        {
        return fmt::format("EntityCreateEvent: Created Entity");
        }

    private:
        Entity m_Entity;
    };

    class EntityDestroyEvent final : public Event
    {
    public:
        DECLARE_EVENT_TYPE(EntityDestroyEvent)

        explicit EntityDestroyEvent(Entity& entity) : m_Entity(entity) {}

        Entity GetEntity() const { return m_Entity; }

        virtual const std::string ToString() const override
        {
            return fmt::format("EntityDestroyEvent: Destroyed Entity");
        }

    private:
        Entity m_Entity;
    };


} // namespace SnowLeopardEngine