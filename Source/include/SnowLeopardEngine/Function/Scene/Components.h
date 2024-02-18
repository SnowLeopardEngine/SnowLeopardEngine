#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsMaterial.h"

namespace SnowLeopardEngine
{
    struct IDComponent
    {
        CoreUUID Id;

        IDComponent()                   = default;
        IDComponent(const IDComponent&) = default;

        std::string GetIdString() const { return to_string(Id); }
        void        SetIdByString(std::string id)
        {
            auto optionalId = CoreUUID::from_string(id);
            if (optionalId.has_value())
            {
                Id = optionalId.value();
            }
        }
    };

    struct NameComponent
    {
        std::string Name;

        NameComponent()                     = default;
        NameComponent(const NameComponent&) = default;
        explicit NameComponent(const std::string& name) : Name(name) {}
    };

    struct TreeNodeComponent
    {
        CoreUUID              ParentId;
        std::vector<CoreUUID> ChildrenIds;

        TreeNodeComponent()                         = default;
        TreeNodeComponent(const TreeNodeComponent&) = default;

        std::string GetParentIdString() const { return to_string(ParentId); }
        void        SetParentIdByString(std::string parentId)
        {
            auto optionalId = CoreUUID::from_string(parentId);
            if (optionalId.has_value())
            {
                ParentId = optionalId.value();
            }
        }

        std::vector<std::string> GetChildrenIdsString()
        {
            std::vector<std::string> result;
            result.reserve(ChildrenIds.size());
            for (const auto& id : ChildrenIds)
            {
                result.emplace_back(to_string(id));
            }
            return result;
        }
        void SetChildrenIdsByStringList(std::vector<std::string> idList)
        {
            ChildrenIds.clear();
            for (const auto& idString : idList)
            {
                auto optionalId = CoreUUID::from_string(idString);
                if (optionalId.has_value())
                {
                    ChildrenIds.emplace_back(optionalId.value());
                }
            }
        }
    };

    struct TransformComponent
    {
        glm::vec3 Position = glm::vec3(0, 0, 0);
        glm::vec3 Scale    = glm::vec3(1, 1, 1);

        TransformComponent()                          = default;
        TransformComponent(const TransformComponent&) = default;
        explicit TransformComponent(const glm::vec3& position, const glm::vec3& scale = glm::vec3(1, 1, 1)) :
            Position(position), Scale(scale)
        {}

    private:
        glm::vec3 m_RotationEuler = glm::vec3(0, 0, 0);
        glm::quat m_Rotation      = glm::quat(1, 0, 0, 0);

    public:
        glm::mat4 GetTransform() const { return Math::GetTransformMatrix(Position, m_Rotation, Scale); }

        glm::vec3 GetRotationEuler() const { return m_RotationEuler; }

        void SetRotationEuler(const glm::vec3& euler)
        {
            m_RotationEuler = euler;
            m_Rotation      = glm::quat(glm::radians(euler));
        }

        glm::quat GetRotation() const { return m_Rotation; }

        void SetRotation(glm::quat rotation)
        {
            m_Rotation      = rotation;
            m_RotationEuler = glm::eulerAngles(rotation);
        }
    };

    struct NativeScriptingComponent
    {
        Ref<NativeScriptInstance> ScriptInstance;

        NativeScriptingComponent()                                = default;
        NativeScriptingComponent(const NativeScriptingComponent&) = default;
        explicit NativeScriptingComponent(const Ref<NativeScriptInstance>& scriptInstance) :
            ScriptInstance(scriptInstance)
        {}
    };

    struct RigidBodyComponent
    {
        float Mass = 1.0f;

        RigidBodyComponent()                          = default;
        RigidBodyComponent(const RigidBodyComponent&) = default;
        explicit RigidBodyComponent(float mass) : Mass(mass) {}
    };

    struct SphereColliderComponent
    {
        float            Radius   = 0.5f;
        PhysicsMaterial* Material = nullptr;

        SphereColliderComponent()                               = default;
        SphereColliderComponent(const SphereColliderComponent&) = default;
        SphereColliderComponent(float radius, PhysicsMaterial* material) : Radius(radius), Material(material) {}
    };
} // namespace SnowLeopardEngine
