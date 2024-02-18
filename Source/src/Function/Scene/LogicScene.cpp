#include "SnowLeopardEngine/Function/Scene/LogicScene.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

#include <fmt/core.h>

namespace SnowLeopardEngine
{
#define ON_COMPONENT_ADDED(comp) \
    template<> \
    void LogicScene::OnComponentAdded<comp>(Entity entity, comp & component)

    LogicScene::LogicScene(const std::string& name) : m_Name(name)
    {
        m_EntityMap = CreateRef<std::map<CoreUUID, Entity>>();
    }

    Entity LogicScene::CreateEntity(const std::string& name)
    {
        CoreUUID uuid = CoreUUIDHelper::CreateStandardUUID();
        Entity   entity(m_Registry.create(), this);
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TreeNodeComponent>();
        entity.AddComponent<TransformComponent>();

        // check duplicate
        auto     createName     = name.empty() ? "Entity" : name;
        uint32_t duplicateCount = m_Name2CountMap.count(createName) > 0 ? m_Name2CountMap[createName] : 0;
        if (duplicateCount > 0)
        {
            // assign a new name, avoid same name
            m_Name2CountMap[createName]++;
            createName = fmt::format("{0} ({1})", createName, duplicateCount);
        }
        else
        {
            m_Name2CountMap[createName] = 1;
        }
        entity.AddComponent<NameComponent>(createName);

        (*m_EntityMap)[uuid] = entity;

        return entity;
    }

    Entity LogicScene::CreateEntityFromContent(CoreUUID uuid, const std::string& name)
    {
        Entity entity {m_Registry.create(), this};

        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<NameComponent>(name);

        (*m_EntityMap)[uuid] = entity;

        return entity;
    }

    void LogicScene::DestroyEntity(Entity entity)
    {
        // Destroy children
        if (entity.HasChildren())
        {
            for (const auto& childUUID : entity.GetChildrenUUIDs())
            {
                DestroyEntity(GetEntityWithCoreUUID(childUUID));
            }
        }

        // If self has parent, let parent remove self
        if (entity.HasParent())
        {
            auto parent = GetEntityWithCoreUUID(entity.GetParentUUID());
            parent.RemoveChild(entity.GetCoreUUID());
        }

        // Destroy self
        m_EntityMap->erase(entity.GetCoreUUID());
        m_Registry.destroy(entity);
    }

    Entity LogicScene::GetEntityWithCoreUUID(CoreUUID id) const { return m_EntityMap->at(id); }

    void LogicScene::OnLoad()
    {
        m_Registry.view<NativeScriptingComponent>().each(
            [](entt::entity entity, NativeScriptingComponent& nativeScript) { nativeScript.ScriptInstance->OnLoad(); });
    }

    void LogicScene::OnTick(float deltaTime)
    {
        // Tick NativeScriptingComponents for now
        // TODO: Consider Script Tick Priority
        // TODO: If time is enough, integrate Lua or C# Scripting.
        m_Registry.view<NativeScriptingComponent>().each(
            [deltaTime](entt::entity entity, NativeScriptingComponent& nativeScript) {
                nativeScript.ScriptInstance->OnTick(deltaTime);
            });
    }

    void LogicScene::OnFixedTick()
    {
        m_Registry.view<NativeScriptingComponent>().each(
            [](entt::entity entity, NativeScriptingComponent& nativeScript) {
                nativeScript.ScriptInstance->OnFixedTick();
            });
    }

    void LogicScene::OnUnload()
    {
        m_Registry.view<NativeScriptingComponent>().each(
            [](entt::entity entity, NativeScriptingComponent& nativeScript) {
                nativeScript.ScriptInstance->OnUnload();
            });
    }

    std::vector<Entity> LogicScene::GetEntitiesSortedByName()
    {
        std::vector<Entity> entityVector;
        for (auto [uuid, entity] : *m_EntityMap)
        {
            entityVector.emplace_back(entity);
        }

        std::sort(entityVector.begin(), entityVector.end(), [this](const auto& a, const auto& b) {
            const auto& nameA = GetNameFromEntity(a);
            const auto& nameB = GetNameFromEntity(b);

            int numberA = ExtractEntityNumber(nameA);
            int numberB = ExtractEntityNumber(nameB);

            if (numberA != 0 && numberB != 0)
            {
                return numberA < numberB;
            }

            return nameA < nameB;
        });

        return entityVector;
    }

    std::string LogicScene::GetNameFromEntity(Entity entity) const
    {
        if (auto* nameComponent = m_Registry.try_get<NameComponent>(entity))
        {
            return nameComponent->Name;
        }
        else
        {
            return "Untitled Entity";
        }
    }

    int LogicScene::ExtractEntityNumber(const std::string& name)
    {
        std::smatch match;
        if (std::regex_search(name, match, std::regex {R"((\d+))"}))
        {
            return std::stoi(match[1]);
        }
        return 0;
    }

    template<typename T>
    void LogicScene::OnComponentAdded(Entity entity, T& component)
    {}

    ON_COMPONENT_ADDED(IDComponent) {}
    ON_COMPONENT_ADDED(NameComponent) {}
    ON_COMPONENT_ADDED(TreeNodeComponent) {}
    ON_COMPONENT_ADDED(TransformComponent) {}
    ON_COMPONENT_ADDED(NativeScriptingComponent) {}
    ON_COMPONENT_ADDED(RigidBodyComponent) {}
    ON_COMPONENT_ADDED(SphereColliderComponent) {}
} // namespace SnowLeopardEngine
