#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/Geometry/HeightMap.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsMaterial.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Texture.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

#include <PxPhysicsAPI.h>

namespace SnowLeopardEngine
{
    // -------- Entity Built-in Components DEFINITION START --------

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

    struct EntityStatusComponent
    {
        bool IsStatic = false;

        EntityStatusComponent()                             = default;
        EntityStatusComponent(const EntityStatusComponent&) = default;
        explicit EntityStatusComponent(bool isStatic) : IsStatic(isStatic) {}
    };

    // -------- Entity Built-in Components DEFINITION END --------

    // -------- Scripting Components DEFINITION START --------

    struct NativeScriptingComponent
    {
        Ref<NativeScriptInstance> ScriptInstance;

        NativeScriptingComponent()                                = default;
        NativeScriptingComponent(const NativeScriptingComponent&) = default;
        explicit NativeScriptingComponent(const Ref<NativeScriptInstance>& scriptInstance) :
            ScriptInstance(scriptInstance)
        {}
    };

    // -------- Scripting Components DEFINITION END --------

    // -------- Physics Components DEFINITION START --------

    // TODO: Jubiao Lin Add more properties here
    // https://github.com/SnowLeopardEngine/SnowLeopardEngine/issues/10
    struct RigidBodyComponent
    {
        float Mass           = 1.0f;
        bool  EnableCCD      = false;
        float LinearDamping  = 0.0f;
        float AngularDamping = 0.05f;

        physx::PxRigidActor* InternalBody = nullptr;

        RigidBodyComponent()                          = default;
        RigidBodyComponent(const RigidBodyComponent&) = default;
        explicit RigidBodyComponent(float mass) : Mass(mass) {}
    };

    struct SphereColliderComponent
    {
        float                Radius    = 0.0f;
        Ref<PhysicsMaterial> Material  = nullptr;
        bool                 IsTrigger = false;

        SphereColliderComponent()                               = default;
        SphereColliderComponent(const SphereColliderComponent&) = default;
        explicit SphereColliderComponent(const Ref<PhysicsMaterial>& material, bool isTrigger = false) :
            Material(material), IsTrigger(isTrigger)
        {}
        explicit SphereColliderComponent(float                       radius,
                                         const Ref<PhysicsMaterial>& material  = nullptr,
                                         bool                        isTrigger = false) :
            Radius(radius),
            Material(material), IsTrigger(isTrigger)
        {}
    };

    struct BoxColliderComponent
    {
        glm::vec3            Offset    = {0, 0, 0};
        glm::vec3            Size      = {0, 0, 0};
        Ref<PhysicsMaterial> Material  = nullptr;
        bool                 IsTrigger = false;

        BoxColliderComponent()                            = default;
        BoxColliderComponent(const BoxColliderComponent&) = default;
        explicit BoxColliderComponent(const Ref<PhysicsMaterial>& material, bool isTrigger = false) :
            Material(material), IsTrigger(isTrigger)
        {}
        explicit BoxColliderComponent(const glm::vec3&            size,
                                      const glm::vec3&            offset    = {0, 0, 0},
                                      const Ref<PhysicsMaterial>& material  = nullptr,
                                      bool                        isTrigger = false) :
            Offset(offset),
            Size(size), Material(material), IsTrigger(isTrigger)
        {}
    };

    struct CapsuleColliderComponent
    {
        float                Radius    = 0.5f;
        float                Height    = 1.0f;
        glm::vec3            Offset    = {0, 0, 0};
        Ref<PhysicsMaterial> Material  = nullptr;
        bool                 IsTrigger = false;

        CapsuleColliderComponent()                                = default;
        CapsuleColliderComponent(const CapsuleColliderComponent&) = default;
        explicit CapsuleColliderComponent(float                      radius,
                                          float                      height,
                                          const glm::vec3&           offset    = {0, 0, 0},
                                          const Ref<PhysicsMaterial> material  = nullptr,
                                          bool                       isTrigger = false) :
            Radius(radius),
            Height(height), Offset(offset), Material(material), IsTrigger(isTrigger)
        {}
    };

    struct TerrainColliderComponent
    {
        Ref<PhysicsMaterial> Material  = nullptr;
        bool                 IsTrigger = false;

        physx::PxRigidStatic* InternalBody = nullptr;

        TerrainColliderComponent()                                = default;
        TerrainColliderComponent(const TerrainColliderComponent&) = default;
        explicit TerrainColliderComponent(const Ref<PhysicsMaterial>& material, bool isTrigger = false) :
            Material(material), IsTrigger(isTrigger)
        {}
    };

    // -------- Physics Components DEFINITION END --------

    // -------- Rendering Components DEFINITION START --------

    enum class CameraClearFlags
    {
        Color = 0,
        Skybox,
    };

    enum class CameraProjection
    {
        Perspective = 0,
        Orthographic,
    };

    struct CameraComponent
    {
        CameraClearFlags ClearFlags  = CameraClearFlags::Color;
        glm::vec4        ClearColor  = glm::vec4(0.192157f, 0.301961f, 0.47451f, 1.0f);
        CameraProjection Projection  = CameraProjection::Perspective;
        float            FOV         = 60.0f;
        float            Near        = 0.1f;
        float            Far         = 1000.0f;
        float            AspectRatio = 16.0f / 9.0f;

        std::vector<std::filesystem::path> CubemapFilePaths;
        Ref<Texture3D>                     Cubemap = nullptr;

        CameraComponent()                       = default;
        CameraComponent(const CameraComponent&) = default;
    };

    struct FreeMoveCameraControllerComponent
    {
        float Sensitivity = 0.05f;
        float Speed       = 0.1f;

        bool      IsFirstTime = true;
        glm::vec2 LastFrameMousePosition;

        FreeMoveCameraControllerComponent()                                         = default;
        FreeMoveCameraControllerComponent(const FreeMoveCameraControllerComponent&) = default;
    };

    struct DirectionalLightComponent
    {
        glm::vec3 Direction = glm::normalize(glm::vec3(-0.6, -1, -1.2));
        float     Intensity = 0.8;
        glm::vec3 Color     = {1, 0.996, 0.885};

        DirectionalLightComponent()                                 = default;
        DirectionalLightComponent(const DirectionalLightComponent&) = default;
    };

    struct MeshFilterComponent
    {
        // TODO: Remove, add AssetManager
        std::filesystem::path FilePath;
        MeshPrimitiveType     PrimitiveType;

        MeshGroup Meshes;

        MeshFilterComponent()                           = default;
        MeshFilterComponent(const MeshFilterComponent&) = default;

        void AssignEntityID(int entityID)
        {
            for (auto& meshItem : Meshes.Items)
            {
                for (auto& vertex : meshItem.Data.Vertices)
                {
                    vertex.EntityID = entityID;
                }
            }
        }
    };

    struct MeshRendererComponent
    {
        glm::vec4 BaseColor;

        // TODO: Add MaterialSystem & other stuff
        bool                  UseDiffuse = false;
        std::filesystem::path DiffuseTextureFilePath;
        Ref<Texture2D>        DiffuseTexture = nullptr;

        bool CastShadow = true;

        MeshRendererComponent()                             = default;
        MeshRendererComponent(const MeshRendererComponent&) = default;
    };

    struct TerrainComponent
    {
        HeightMap TerrainHeightMap;

        float XScale = 1;
        float YScale = 1;
        float ZScale = 1;

        MeshItem Mesh;

        TerrainComponent() = default;
        explicit TerrainComponent(const HeightMap& heightMap) : TerrainHeightMap(heightMap) {}
        TerrainComponent(const TerrainComponent&) = default;
    };

    struct TerrainRendererComponent
    {
        glm::vec4 BaseColor;

        // TODO: Add MaterialSystem & other stuff
        bool                  UseDiffuse = false;
        std::filesystem::path DiffuseTextureFilePath;
        Ref<Texture2D>        DiffuseTexture = nullptr;

        bool CastShadow = true;

        TerrainRendererComponent()                                = default;
        TerrainRendererComponent(const TerrainRendererComponent&) = default;
    };
    // -------- Rendering Components DEFINITION END --------
} // namespace SnowLeopardEngine
