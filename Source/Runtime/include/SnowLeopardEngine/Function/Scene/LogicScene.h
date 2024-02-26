#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"

#include <entt/entt.hpp>

namespace SnowLeopardEngine
{
    class Entity;

    class LogicScene
    {
    public:
        LogicScene(const std::string& name = "Untitled Scene");
        ~LogicScene() = default;

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

        friend class Entity;
    };
} // namespace SnowLeopardEngine
