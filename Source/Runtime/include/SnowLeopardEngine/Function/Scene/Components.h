#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"
#include "SnowLeopardEngine/Function/Animation/AnimatorController.h"
#include "SnowLeopardEngine/Function/GUI/GUITypeDef.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/Geometry/HeightMap.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsMaterial.h"
#include "SnowLeopardEngine/Function/Rendering/Material.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Scene/LayerManager.h"
#include "SnowLeopardEngine/Function/Scene/TagManager.h"

#include "cereal/cereal.hpp"
#include <PxPhysicsAPI.h>
#include <entt/fwd.hpp>

// CppAst.NET Macro
#if !defined(__cppast)
#define __cppast(...)
#endif

#define COMPONENT_NAME(comp) \
    static std::string GetName() { return #comp; }

namespace SnowLeopardEngine
{
    // -------- Entity Built-in Components DEFINITION START --------
    struct __cppast(smeta_unit) IDComponent
    {
        COMPONENT_NAME(ID)

        // clang-format off
        __cppast(getter = GetIdString)
        __cppast(setter = SetIdByString)
        __cppast(tooltip = "UUID of an entity")
        CoreUUID Id;
        // clang-format on

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(Id));
        }
        // NOLINTEND

        IDComponent() = default;
        explicit IDComponent(CoreUUID id) : Id(id) {}
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

    struct __cppast(smeta_unit) NameComponent
    {
        COMPONENT_NAME(Name)

        __cppast(tooltip = "Name of an entity") std::string Name;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(Name));
        }
        // NOLINTEND

        NameComponent()                     = default;
        NameComponent(const NameComponent&) = default;
        explicit NameComponent(const std::string& name) : Name(name) {}
    };

    struct __cppast(smeta_unit) TagComponent
    {
        COMPONENT_NAME(Tag)

        __cppast(tooltip = "Tag of an entity") std::string TagValue = Tag::Untagged;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(TagValue));
        }
        // NOLINTEND

        TagComponent()                    = default;
        TagComponent(const TagComponent&) = default;
    };

    struct __cppast(smeta_unit) LayerComponent
    {
        COMPONENT_NAME(Layer)

        __cppast(tooltip = "Layer of an entity") Layer LayerValue = static_cast<Layer>(LayerMask::Default);

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(LayerValue));
        }
        // NOLINTEND

        LayerComponent()                      = default;
        LayerComponent(const LayerComponent&) = default;
    };

    struct __cppast(smeta_unit) TreeNodeComponent
    {
        COMPONENT_NAME(TreeNode)

        // clang-format off
        __cppast(getter = GetParentIdString)
        __cppast(setter = SetParentIdByString)
        __cppast(tooltip = "UUID of parent ")
        CoreUUID ParentId;
        // clang-format on

        // clang-format off
        __cppast(getter = GetChildrenIdsString)
        __cppast(setter = SetChildrenIdsByStringList)
        __cppast(tooltip = "UUIDs of children")
        std::vector<CoreUUID> ChildrenIds;
        // clang-format on

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(ParentId), CEREAL_NVP(ChildrenIds));
        }
        // NOLINTEND

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

    struct __cppast(smeta_unit) TransformComponent
    {
        COMPONENT_NAME(Transform)

        __cppast(tooltip = "Position of an entity") glm::vec3 Position = glm::vec3(0, 0, 0);

        __cppast(tooltip = "Scale of an entity") glm::vec3 Scale = glm::vec3(1, 1, 1);

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(Position), cereal::make_nvp("Rotation", m_Rotation), CEREAL_NVP(Scale));
        }
        // NOLINTEND

        TransformComponent()                          = default;
        TransformComponent(const TransformComponent&) = default;
        explicit TransformComponent(const glm::vec3& position, const glm::vec3& scale = glm::vec3(1, 1, 1)) :
            Position(position), Scale(scale)
        {}

    private:
        // clang-format off
        __cppast(getter = GetRotationEuler)
        __cppast(setter = SetRotationEuler)
        __cppast(tooltip = "Euler angles rotation of an entity")
        glm::vec3 m_RotationEuler = glm::vec3(0, 0, 0);
        // clang-format on

        // clang-format off
        __cppast(getter = GetRotation)
        __cppast(setter = SetRotation)
        __cppast(tooltip = "Quaternion rotation of an entity")
        glm::quat m_Rotation = glm::quat(1, 0, 0, 0);
        // clang-format on

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

    struct __cppast(smeta_unit) EntityStatusComponent
    {
        COMPONENT_NAME(EntityStatus)

        __cppast(tooltip = "Is the entity enabled") bool IsEnabled = true;
        __cppast(tooltip = "Is the entity static") bool  IsStatic  = false;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(IsEnabled), CEREAL_NVP(IsStatic));
        }
        // NOLINTEND

        EntityStatusComponent()                             = default;
        EntityStatusComponent(const EntityStatusComponent&) = default;
        explicit EntityStatusComponent(bool isStatic) : IsStatic(isStatic) {}
    };

    // -------- Entity Built-in Components DEFINITION END --------

    // -------- Scripting Components DEFINITION START --------

    struct __cppast(smeta_unit) NativeScriptingComponent
    {
        COMPONENT_NAME(NativeScripting)

        std::string               ScriptName;
        Ref<NativeScriptInstance> ScriptInstance;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(ScriptName));
        }
        // NOLINTEND

        NativeScriptingComponent()                                = default;
        NativeScriptingComponent(const NativeScriptingComponent&) = default;
        explicit NativeScriptingComponent(std::string scriptName) : ScriptName(scriptName) {}
    };

    // -------- Scripting Components DEFINITION END --------

    // -------- Physics Components DEFINITION START --------

    // TODO: Jubiao Lin Add more properties here
    // https://github.com/SnowLeopardEngine/SnowLeopardEngine/issues/10
    struct __cppast(smeta_unit) RigidBodyComponent
    {
        COMPONENT_NAME(RigidBody)

        __cppast(tooltip = "The mass of the rigidBody") float Mass = 1.0f;

        __cppast(tooltip = "Enable Continuous Collision Detection") bool EnableCCD = false;

        __cppast(tooltip = "The linear damping of the rigidBody") float LinearDamping = 0.0f;

        __cppast(tooltip = "The angular damping of the rigidBody") float AngularDamping = 0.05f;

        __cppast(ignore) physx::PxRigidActor* InternalBody = nullptr;

        __cppast(ignore) bool IsStatic = false;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(Mass), CEREAL_NVP(EnableCCD), CEREAL_NVP(LinearDamping), CEREAL_NVP(AngularDamping));
        }
        // NOLINTEND

        RigidBodyComponent()                          = default;
        RigidBodyComponent(const RigidBodyComponent&) = default;
        explicit RigidBodyComponent(float mass, float linear, float augular, bool ccd) :
            Mass(mass), LinearDamping(linear), AngularDamping(augular), EnableCCD(ccd)
        {}
    };

    struct __cppast(smeta_unit) SphereColliderComponent
    {
        COMPONENT_NAME(SphereCollider)

        __cppast(tooltip = "The radius of the SphereCollider") float Radius = 0.0f;

        __cppast(tooltip = "Is trigger") bool IsTrigger = false;

        __cppast(ignore) Ref<PhysicsMaterial> Material = nullptr;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(Radius), CEREAL_NVP(IsTrigger));
        }
        // NOLINTEND

        SphereColliderComponent()                               = default;
        SphereColliderComponent(const SphereColliderComponent&) = default;
        explicit SphereColliderComponent(const Ref<PhysicsMaterial>& material, bool isTrigger = false) :
            Material(material), IsTrigger(isTrigger)
        {}
        explicit SphereColliderComponent(
            float radius, const Ref<PhysicsMaterial>& material = nullptr, bool isTrigger = false) :
            Radius(radius),
            Material(material), IsTrigger(isTrigger)
        {}
    };

    struct __cppast(smeta_unit) BoxColliderComponent
    {
        COMPONENT_NAME(BoxCollider)

        __cppast(tooltip = "The offset of the BoxCollider") glm::vec3 Offset = {0, 0, 0};

        __cppast(tooltip = "The size of the BoxCollider") glm::vec3 Size = {0, 0, 0};

        __cppast(tooltip = "Is the BoxCollider a trigger") bool IsTrigger = false;

        __cppast(ignore) Ref<PhysicsMaterial> Material = nullptr;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(Offset), CEREAL_NVP(Size), CEREAL_NVP(IsTrigger));
        }
        // NOLINTEND

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

    struct __cppast(smeta_unit) CapsuleColliderComponent
    {
        COMPONENT_NAME(CapsuleCollider)

        __cppast(tooltip = "The radius of the CapsuleCollider's hemisphere") float Radius = 0.5f;

        __cppast(tooltip = "The height of the CapsuleCollider") float Height = 1.0f;

        __cppast(tooltip = "The offset of the CapsuleCollider") glm::vec3 Offset = {0, 0, 0};

        __cppast(tooltip = "Is the CapsuleCollider a trigger") bool IsTrigger = false;

        __cppast(ignore) Ref<PhysicsMaterial> Material = nullptr;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(Radius), CEREAL_NVP(Height), CEREAL_NVP(Offset), CEREAL_NVP(IsTrigger));
        }
        // NOLINTEND

        CapsuleColliderComponent()                                = default;
        CapsuleColliderComponent(const CapsuleColliderComponent&) = default;
        explicit CapsuleColliderComponent(float                       radius,
                                          float                       height,
                                          const glm::vec3&            offset    = {0, 0, 0},
                                          const Ref<PhysicsMaterial>& material  = nullptr,
                                          bool                        isTrigger = false) :
            Radius(radius),
            Height(height), Offset(offset), Material(material), IsTrigger(isTrigger)
        {}
    };

    struct __cppast(smeta_unit) TerrainColliderComponent
    {
        COMPONENT_NAME(TerrainCollider)

        __cppast(tooltip = "Is the TerrainCollider a trigger") bool IsTrigger = false;

        __cppast(ignore) Ref<PhysicsMaterial> Material = nullptr;

        __cppast(ignore) physx::PxRigidStatic* InternalBody = nullptr;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(IsTrigger));
        }
        // NOLINTEND

        TerrainColliderComponent()                                = default;
        TerrainColliderComponent(const TerrainColliderComponent&) = default;
        explicit TerrainColliderComponent(const Ref<PhysicsMaterial>& material, bool isTrigger = false) :
            Material(material), IsTrigger(isTrigger)
        {}
    };

    // TODO: Register more smeta units

    struct __cppast(smeta_unit) CharacterControllerComponent
    {
        COMPONENT_NAME(CharacterController)

        float     SlopeLimit  = 45.0f;
        float     StepOffset  = 0.3;
        float     Height      = 1.0f;
        float     Radius      = 0.5f;
        float     MinMoveDisp = 0.2;
        glm::vec3 Offset      = {0, 0.5, 0};

        Ref<PhysicsMaterial>       Material           = nullptr;
        physx::PxController*       InternalController = nullptr;
        physx::PxControllerFilters Filters;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(SlopeLimit),
                    CEREAL_NVP(StepOffset),
                    CEREAL_NVP(Height),
                    CEREAL_NVP(Radius),
                    CEREAL_NVP(MinMoveDisp),
                    CEREAL_NVP(Offset));
        }
        // NOLINTEND

        CharacterControllerComponent()                                    = default;
        CharacterControllerComponent(const CharacterControllerComponent&) = default;
        explicit CharacterControllerComponent(
            float height, float radius, glm::vec3 offset, float slopeLimit, const Ref<PhysicsMaterial>& material) :
            Height(height),
            Radius(radius), Offset(offset), SlopeLimit(slopeLimit), Material(material)
        {}
    };

    struct MeshColliderComponent
    {
        COMPONENT_NAME(MeshCollider)

        glm::vec3            Offset    = {0, 0, 0};
        Ref<PhysicsMaterial> Material  = nullptr;
        bool                 IsTrigger = false;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Offset), CEREAL_NVP(IsTrigger));
        }
        // NOLINTEND

        MeshColliderComponent()                             = default;
        MeshColliderComponent(const MeshColliderComponent&) = default;
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
        COMPONENT_NAME(Camera)

        CameraClearFlags ClearFlags     = CameraClearFlags::Color;
        glm::vec4        ClearColor     = glm::vec4(0.192157f, 0.301961f, 0.47451f, 1.0f);
        CameraProjection Projection     = CameraProjection::Perspective;
        float            FOV            = 60.0f;
        float            Near           = 0.1f;
        float            Far            = 1000.0f;
        float            ViewportWidth  = 0;
        float            ViewportHeight = 0;
        bool             IsPrimary      = true;

        bool                  IsEnvironmentMapHDR    = true;
        std::filesystem::path EnvironmentMapFilePath = "Assets/Textures/DefaultSky.hdr";

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(ClearFlags),
                    CEREAL_NVP(ClearColor),
                    CEREAL_NVP(Projection),
                    CEREAL_NVP(FOV),
                    CEREAL_NVP(Near),
                    CEREAL_NVP(Far),
                    CEREAL_NVP(IsPrimary),
                    CEREAL_NVP(EnvironmentMapFilePath));
        }
        // NOLINTEND

        CameraComponent()                       = default;
        CameraComponent(const CameraComponent&) = default;
    };

    struct FreeMoveCameraControllerComponent
    {
        COMPONENT_NAME(FreeMoveCameraController)

        float Sensitivity = 0.05f;
        float Speed       = 0.1f;

        bool      IsFirstTime = true;
        glm::vec2 LastFrameMousePosition;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Sensitivity),
                    CEREAL_NVP(Speed),
                    CEREAL_NVP(IsFirstTime),
                    CEREAL_NVP(LastFrameMousePosition));
        }
        // NOLINTEND

        FreeMoveCameraControllerComponent()                                         = default;
        FreeMoveCameraControllerComponent(const FreeMoveCameraControllerComponent&) = default;
    };

    struct ThirdPersonFollowCameraControllerComponent
    {
        float Sensitivity = 0.05f;
        float Speed       = 0.1f;

        glm::vec3 Offset = {0, 20, 45};

        entt::entity FollowEntity;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Sensitivity),
                    CEREAL_NVP(Speed),
                    CEREAL_NVP(Offset),
                    CEREAL_NVP(FollowEntity));
        }
        // NOLINTEND

        ThirdPersonFollowCameraControllerComponent()                                                  = default;
        ThirdPersonFollowCameraControllerComponent(const ThirdPersonFollowCameraControllerComponent&) = default;
    };

    struct DirectionalLightComponent
    {
        COMPONENT_NAME(DirectionalLight)

        glm::vec3 Direction = glm::normalize(glm::vec3(-0.6, -1, -1.2));
        float     Intensity = 5.0;
        glm::vec3 Color     = {1, 0.996, 0.885};

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Direction), CEREAL_NVP(Intensity), CEREAL_NVP(Color));
        }
        // NOLINTEND

        DirectionalLightComponent()                                 = default;
        DirectionalLightComponent(const DirectionalLightComponent&) = default;
    };

    struct PointLightComponent
    {
        COMPONENT_NAME(PointLight)

        glm::vec3 Color = {1, 0.996, 0.885};

        float Constant  = 1.0f;
        float Linear    = 0.09f;
        float Quadratic = 0.032f;
        float Intensity = 10.0;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Color),
                    CEREAL_NVP(Constant),
                    CEREAL_NVP(Linear),
                    CEREAL_NVP(Quadratic),
                    CEREAL_NVP(Intensity));
        }
        // NOLINTEND

        PointLightComponent()                           = default;
        PointLightComponent(const PointLightComponent&) = default;
    };

    struct MeshFilterComponent
    {
        COMPONENT_NAME(MeshFilter)

        std::filesystem::path FilePath;

        bool              UsePrimitive  = false;
        MeshPrimitiveType PrimitiveType = MeshPrimitiveType::Invalid;

        MeshGroup* Meshes;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(FilePath), CEREAL_NVP(UsePrimitive), CEREAL_NVP(PrimitiveType));
        }
        // NOLINTEND

        MeshFilterComponent()                           = default;
        MeshFilterComponent(const MeshFilterComponent&) = default;

        ~MeshFilterComponent()
        {
            if (UsePrimitive)
            {
                delete Meshes;
                Meshes = nullptr;
            }
        }

        void AssignEntityID(int entityID) const
        {
            for (auto& meshItem : Meshes->Items)
            {
                for (auto& vertex : meshItem.Data.Vertices)
                {
                    vertex.EntityID = entityID;
                }
            }
        }
    };

    struct BaseRendererComponent
    {
        COMPONENT_NAME(BaseRenderer)

        bool CastShadow = true;

        std::filesystem::path MaterialFilePath;
        Material*             Mat = nullptr;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(CastShadow), CEREAL_NVP(MaterialFilePath));
        }
        // NOLINTEND

        BaseRendererComponent()                             = default;
        BaseRendererComponent(const BaseRendererComponent&) = default;
    };

    struct MeshRendererComponent : public BaseRendererComponent
    {
        COMPONENT_NAME(MeshRenderer)

        bool EnableInstancing = false;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            BaseRendererComponent::serialize(archive);
            archive(CEREAL_NVP(EnableInstancing));
        }
        // NOLINTEND

        MeshRendererComponent()                             = default;
        MeshRendererComponent(const MeshRendererComponent&) = default;
    };

    struct TerrainComponent
    {
        COMPONENT_NAME(Terrain)

        HeightMap TerrainHeightMap;

        float XScale = 1;
        float YScale = 1;
        float ZScale = 1;

        MeshItem Mesh;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(XScale), CEREAL_NVP(YScale), CEREAL_NVP(ZScale));
        }
        // NOLINTEND

        TerrainComponent() = default;
        explicit TerrainComponent(const HeightMap& heightMap) : TerrainHeightMap(heightMap) {}
        TerrainComponent(const TerrainComponent&) = default;
    };

    struct TerrainRendererComponent : public BaseRendererComponent
    {
        COMPONENT_NAME(TerrainRenderer)

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            BaseRendererComponent::serialize(archive);
        }
        // NOLINTEND

        TerrainRendererComponent()                                = default;
        TerrainRendererComponent(const TerrainRendererComponent&) = default;
    };
    // -------- Rendering Components DEFINITION END --------

    // -------- Animation Components DEFINITION START --------
    struct AnimatorComponent
    {
        COMPONENT_NAME(Animator)

        AnimatorController Controller;

        AnimatorComponent()                         = default;
        AnimatorComponent(const AnimatorComponent&) = default;
    };
    // -------- Animation Components DEFINITION END --------

    // -------- In-Game GUI Components DEFINITION START --------
    namespace UI
    {
        struct CanvasComponent
        {
            COMPONENT_NAME(CanvasComponent)

            CoreUUID CanvasCameraUUID;

            // NOLINTBEGIN
            template<class Archive>
            void serialize(Archive& archive)
            {
                archive(CEREAL_NVP(CanvasCameraUUID));
            }
            // NOLINTEND

            CanvasComponent()                       = default;
            CanvasComponent(const CanvasComponent&) = default;
        };

        struct RectTransformComponent
        {
            COMPONENT_NAME(RectTransformComponent)

            glm::vec3 Pos;
            float     RotationAngle = 0;
            glm::vec2 Size;
            glm::vec2 Pivot = {0.5, 0.5};

            // NOLINTBEGIN
            template<class Archive>
            void serialize(Archive& archive)
            {
                archive(CEREAL_NVP(Pos), CEREAL_NVP(RotationAngle), CEREAL_NVP(Size), CEREAL_NVP(Pivot));
            }
            // NOLINTEND

            RectTransformComponent()                              = default;
            RectTransformComponent(const RectTransformComponent&) = default;
        };

        struct ButtonComponent
        {
            UI::ButtonTintType TintType = UI::ButtonTintType::Color;

            UI::ColorTint   TintColor;
            UI::TextureTint TintTexture;

            MeshItem ImageMesh = GeometryFactory::CreateMeshPrimitive<QuadMesh>(true);

            std::filesystem::path MaterialFilePath;
            Material*             Mat = nullptr;

            // NOLINTBEGIN
            template<class Archive>
            void serialize(Archive& archive)
            {
                archive(CEREAL_NVP(TintType), CEREAL_NVP(TintColor), CEREAL_NVP(MaterialFilePath));
            }
            // NOLINTEND

            ButtonComponent()                       = default;
            ButtonComponent(const ButtonComponent&) = default;
        };

        struct ImageComponent
        {
            std::filesystem::path TargetGraphicPath;
            RenderTarget          TargetGraphic = nullptr;

            glm::vec4 Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

            MeshItem ImageMesh = GeometryFactory::CreateMeshPrimitive<QuadMesh>(true);

            std::filesystem::path MaterialFilePath;
            Material*             Mat = nullptr;

            // NOLINTBEGIN
            template<class Archive>
            void serialize(Archive& archive)
            {
                archive(CEREAL_NVP(TargetGraphicPath), CEREAL_NVP(Color), CEREAL_NVP(MaterialFilePath));
            }
            // NOLINTEND

            ImageComponent()                      = default;
            ImageComponent(const ImageComponent&) = default;
        };

        struct TextComponent
        {
            std::string  TextContent;
            std::string  FontFilePath;
            unsigned int FontSize = 12;
            glm::vec4    Color    = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            enum Alignment
            {
                Left,
                Center,
                Right
            } TextAlignment = Center;

            MeshItem Mesh = GeometryFactory::CreateMeshPrimitive<QuadMesh>(true);

            std::filesystem::path MaterialFilePath;
            Material*             Mat = nullptr;

            // NOLINTBEGIN
            template<class Archive>
            void serialize(Archive& archive)
            {
                archive(CEREAL_NVP(TextContent),
                        CEREAL_NVP(FontFilePath),
                        CEREAL_NVP(FontSize),
                        CEREAL_NVP(Color),
                        CEREAL_NVP(TextAlignment));
            }
            // NOLINTEND

            TextComponent()                     = default;
            TextComponent(const TextComponent&) = default;
        };
    }; // namespace UI
    // -------- In-Game GUI Components DEFINITION END --------

    template<typename... Component>
    struct ComponentGroup
    {};

#define COMMON_COMPONENT_TYPES \
    TagComponent, LayerComponent, TreeNodeComponent, TransformComponent, EntityStatusComponent, \
        NativeScriptingComponent, RigidBodyComponent, SphereColliderComponent, BoxColliderComponent, \
        CapsuleColliderComponent, TerrainColliderComponent, CharacterControllerComponent, MeshColliderComponent, \
        CameraComponent, FreeMoveCameraControllerComponent, ThirdPersonFollowCameraControllerComponent, \
        DirectionalLightComponent, PointLightComponent, BaseRendererComponent, MeshFilterComponent, \
        MeshRendererComponent, TerrainComponent, TerrainRendererComponent, UI::CanvasComponent, \
        UI::RectTransformComponent, UI::ButtonComponent, UI::ImageComponent, UI::TextComponent

#define ALL_SERIALIZABLE_COMPONENT_TYPES COMMON_COMPONENT_TYPES, IDComponent, NameComponent

#define ALL_COPYABLE_COMPONENT_TYPES COMMON_COMPONENT_TYPES, AnimatorComponent

    using AllCopyableComponents = ComponentGroup<ALL_COPYABLE_COMPONENT_TYPES>;

} // namespace SnowLeopardEngine
