#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"

#include <entt/entt.hpp>

namespace SnowLeopardEngine
{
    class Entity;

    enum class LogicSceneSimulationMode
    {
        Invalid = 0,
        Game,
        Editor
    };

    enum class LogicSceneSimulationStatus
    {
        Invalid = 0,
        Stopped,
        Simulating,
        Paused
    };

    class LogicScene
    {
    public:
        explicit LogicScene(const std::string& name = "Untitled Scene");
        ~LogicScene() = default;

        static Ref<LogicScene> Copy(const Ref<LogicScene>& other);

        void SetSimulationMode(LogicSceneSimulationMode mode)
        {
            m_SimulationMode = mode;
            if (mode == LogicSceneSimulationMode::Editor)
            {
                m_SimulationStatus = LogicSceneSimulationStatus::Stopped;
            }
        }
        LogicSceneSimulationMode GetSimulationMode() const { return m_SimulationMode; }

        void SetSimulationStatus(LogicSceneSimulationStatus status)
        {
            switch (m_SimulationStatus)
            {
                case LogicSceneSimulationStatus::Stopped:
                case LogicSceneSimulationStatus::Paused:
                    if (status == LogicSceneSimulationStatus::Simulating)
                    {
                        m_SimulationStatus = status;
                    }
                    break;

                case LogicSceneSimulationStatus::Simulating:
                    if (status == LogicSceneSimulationStatus::Stopped || status == LogicSceneSimulationStatus::Paused)
                    {
                        m_SimulationStatus = status;
                    }
                    break;

                case LogicSceneSimulationStatus::Invalid:
                    break;
            }
        }
        LogicSceneSimulationStatus GetSimulationStatus() const { return m_SimulationStatus; }

        const std::string& GetName() { return m_Name; }
        std::string        GetFileName() { return m_Name + ".scene"; }

        entt::registry& GetRegistry() { return m_Registry; }

        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityFromContent(CoreUUID uuid, const std::string& name = std::string());
        void   DestroyEntity(Entity entity);
        Entity GetEntityWithCoreUUID(CoreUUID uuid) const;

        void OnLoad();
        void OnTick(float deltaTime);
        void OnFixedTick();
        void OnUnload();

        std::vector<Entity> GetEntitiesSortedByName();

    private:
        std::string GetNameFromEntity(Entity entity) const;
        int         ExtractEntityNumber(const std::string& name);

        template<typename T>
        void OnComponentAdded(Entity entity, T& component);

    private:
        std::string                               m_Name;
        entt::registry                            m_Registry;
        Ref<std::map<CoreUUID, Entity>>           m_EntityMap;
        std::unordered_map<std::string, uint32_t> m_Name2CountMap;

        LogicSceneSimulationStatus m_SimulationStatus = LogicSceneSimulationStatus::Simulating;
        LogicSceneSimulationMode   m_SimulationMode   = LogicSceneSimulationMode::Game;

        friend class Entity;
    };
} // namespace SnowLeopardEngine
