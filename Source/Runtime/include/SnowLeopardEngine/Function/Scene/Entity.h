#pragma once

#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

namespace SnowLeopardEngine
{
    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, LogicScene* scene) : m_EntityHandle(handle), m_Scene(scene) {}
        Entity(const Entity& other) = default;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            SNOW_LEOPARD_CORE_ASSERT(!HasComponent<T>(), "[Entity] Can't add a duplicate component!");
            T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template<typename T, typename... Args>
        T& AddOrReplaceComponent(Args&&... args)
        {
            T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template<typename T>
        T& GetComponent()
        {
            SNOW_LEOPARD_CORE_ASSERT(HasComponent<T>(), "[Entity] Entity does not have any components!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent()
        {
            return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent()
        {
            SNOW_LEOPARD_CORE_ASSERT(HasComponent<T>(), "[Entity] Entity does not have any components!");
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

        operator bool() const { return m_EntityHandle != entt::null; }

        operator entt::entity() const { return m_EntityHandle; }

        operator uint32_t() const { return (uint32_t)m_EntityHandle; }

        CoreUUID GetCoreUUID() { return GetComponent<IDComponent>().Id; }

        const std::string& GetName() { return GetComponent<NameComponent>().Name; }

        void SetParent(Entity& parent)
        {
            auto& treeNodeComponent    = GetComponent<TreeNodeComponent>();
            treeNodeComponent.ParentId = parent.GetCoreUUID();
            parent.GetComponent<TreeNodeComponent>().ChildrenIds.emplace_back(GetCoreUUID());
        }

        bool                         HasParent() { return !GetComponent<TreeNodeComponent>().ParentId.is_nil(); }
        CoreUUID                     GetParentUUID() { return GetComponent<TreeNodeComponent>().ParentId; }
        bool                         HasChildren() { return !GetComponent<TreeNodeComponent>().ChildrenIds.empty(); }
        const std::vector<CoreUUID>& GetChildrenUUIDs() { return GetComponent<TreeNodeComponent>().ChildrenIds; }

        void AddChild(CoreUUID childUUID) { GetComponent<TreeNodeComponent>().ChildrenIds.emplace_back(childUUID); }
        void RemoveChild(CoreUUID childUUID)
        {
            auto& children = GetComponent<TreeNodeComponent>().ChildrenIds;
            auto  it       = std::find(children.begin(), children.end(), childUUID);

            if (it != children.end())
            {
                children.erase(it);
            }
        }

        bool operator==(const Entity& other) const
        {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }

        bool operator!=(const Entity& other) const { return !(*this == other); }

    private:
        entt::entity m_EntityHandle = entt::null;
        LogicScene*  m_Scene        = nullptr;
    };
} // namespace SnowLeopardEngine
