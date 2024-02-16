#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"

namespace SnowLeopardEngine
{
    struct IDComponent
    {
        CoreUUID Id;

        IDComponent()                   = default;
        IDComponent(const IDComponent&) = default;

        std::string GetIdString() { return to_string(Id); }
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
        NameComponent(const std::string& name) : Name(name) {}
    };

    struct TreeNodeComponent
    {
        CoreUUID              ParentId;
        std::vector<CoreUUID> ChildrenIds;

        TreeNodeComponent()                         = default;
        TreeNodeComponent(const TreeNodeComponent&) = default;

        std::string GetParentIdString() { return to_string(ParentId); }
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
        TransformComponent(const glm::vec3& position, const glm::vec3& scale = glm::vec3(1, 1, 1)) :
            Position(position), Scale(scale)
        {}

    private:
        glm::vec3 RotationEuler = glm::vec3(0, 0, 0);
        glm::quat Rotation      = glm::quat(1, 0, 0, 0);

    public:
        glm::mat4 GetTransform() const { return Math::GetTransformMatrix(Position, Rotation, Scale); }

        glm::vec3 GetRotationEuler() const { return RotationEuler; }

        void SetRotationEuler(const glm::vec3& euler)
        {
            RotationEuler = euler;
            Rotation      = glm::quat(glm::radians(euler));
        }

        glm::quat GetRotation() const { return Rotation; }

        void SetRotation(glm::quat rotation)
        {
            Rotation      = rotation;
            RotationEuler = glm::eulerAngles(rotation);
        }
    };

    struct NativeScriptingComponent
    {
        Ref<NativeScriptInstance> ScriptInstance;

        NativeScriptingComponent()                                = default;
        NativeScriptingComponent(const NativeScriptingComponent&) = default;
        NativeScriptingComponent(const Ref<NativeScriptInstance>& scriptInstance) : ScriptInstance(scriptInstance) {}
    };
} // namespace SnowLeopardEngine
