#include "SnowLeopardEngine/Function/Scene/LogicScene.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Animation/Animator.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/IO/MaterialLoader.h"
#include "SnowLeopardEngine/Function/IO/Serialization.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

namespace SnowLeopardEngine
{
#define ON_COMPONENT_ADDED(comp) \
    template<> \
    void LogicScene::OnComponentAdded<comp>(Entity entity, comp & component)

    template<typename... Component>
    static void CopyComponent(entt::registry& dst, entt::registry& src, Ref<std::map<CoreUUID, Entity>>& enttMap)
    {
        (
            [&]() {
                auto view = src.view<Component>();
                for (auto srcEntity : view)
                {
                    Entity dstEntity = enttMap->at(src.get<IDComponent>(srcEntity).Id);

                    auto& srcComponent = src.get<Component>(srcEntity);
                    dst.emplace_or_replace<Component>(dstEntity, srcComponent);
                }
            }(),
            ...);
    }

    template<typename... Component>
    static void CopyComponent(ComponentGroup<Component...>,
                              entt::registry&                  dst,
                              entt::registry&                  src,
                              Ref<std::map<CoreUUID, Entity>>& enttMap)
    {
        CopyComponent<Component...>(dst, src, enttMap);
    }

    template<typename... Component>
    static void CopyComponentIfExists(Entity dst, Entity src)
    {
        (
            [&]() {
                if (src.HasComponent<Component>())
                    dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
            }(),
            ...);
    }

    template<typename... Component>
    static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
    {
        CopyComponentIfExists<Component...>(dst, src);
    }

    Ref<LogicScene> LogicScene::Copy(const Ref<LogicScene>& other)
    {
        Ref<LogicScene> newScene     = CreateRef<LogicScene>(other->GetName() + " (Copy)", true);
        newScene->m_SimulationMode   = other->m_SimulationMode;
        newScene->m_SimulationStatus = other->m_SimulationStatus;
        newScene->m_Name2CountMap    = other->m_Name2CountMap;

        auto& srcSceneRegistry = other->m_Registry;
        auto& dstSceneRegistry = newScene->m_Registry;

        // Create entities in new scene
        auto idView = srcSceneRegistry.view<IDComponent>();
        for (auto e : idView)
        {
            CoreUUID    uuid               = srcSceneRegistry.get<IDComponent>(e).Id;
            const auto& name               = srcSceneRegistry.get<NameComponent>(e).Name;
            Entity      newEntity          = newScene->CreateEntityFromContent(uuid, name);
            (*newScene->m_EntityMap)[uuid] = newEntity;
        }

        // Copy components (except IDComponent and NameComponent)
        CopyComponent(AllCopyableComponents {}, dstSceneRegistry, srcSceneRegistry, newScene->m_EntityMap);

        return newScene;
    }

    LogicScene::LogicScene(const std::string& name, bool copy) : m_Name(name)
    {
        m_EntityMap = CreateRef<std::map<CoreUUID, Entity>>();

        if (!copy)
        {
            CreateDefaultEntities();
        }
    }

    Entity LogicScene::CreateEntity(const std::string& name)
    {
        CoreUUID uuid = CoreUUIDHelper::CreateStandardUUID();
        Entity   entity(m_Registry.create(), this);
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TagComponent>();
        entity.AddComponent<LayerComponent>();
        entity.AddComponent<TreeNodeComponent>();
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<EntityStatusComponent>();

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
        entity.AddComponent<TagComponent>();
        entity.AddComponent<LayerComponent>();
        entity.AddComponent<TreeNodeComponent>();
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<EntityStatusComponent>();

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
        SNOW_LEOPARD_PROFILE_FUNCTION

        LogicScenePreLoadEvent preloadEvent(this);
        TriggerEvent(preloadEvent);

        // Mesh Loading (dirty code for now)
        m_Registry.view<MeshFilterComponent>().each([this](entt::entity entity, MeshFilterComponent& meshFilter) {
            // TODO: Move to AssetManager
            if (meshFilter.PrimitiveType != MeshPrimitiveType::Invalid)
            {
                meshFilter.Meshes = new MeshGroup();

                switch (meshFilter.PrimitiveType)
                {
                    case MeshPrimitiveType::Quad: {
                        auto meshItem = GeometryFactory::CreateMeshPrimitive<QuadMesh>();
                        meshFilter.Meshes->Items.emplace_back(meshItem);
                        break;
                    }

                    case MeshPrimitiveType::Cube: {
                        auto meshItem = GeometryFactory::CreateMeshPrimitive<CubeMesh>();
                        meshFilter.Meshes->Items.emplace_back(meshItem);
                        break;
                    }

                    case MeshPrimitiveType::Sphere: {
                        auto meshItem = GeometryFactory::CreateMeshPrimitive<SphereMesh>();
                        meshFilter.Meshes->Items.emplace_back(meshItem);
                        break;
                    }
                    case MeshPrimitiveType::Capsule: {
                        auto meshItem = GeometryFactory::CreateMeshPrimitive<CapsuleMesh>();
                        meshFilter.Meshes->Items.emplace_back(meshItem);
                        break;
                    }
                    break;
                    case MeshPrimitiveType::Heightfield: {
                        auto meshItem = GeometryFactory::CreateMeshPrimitive<HeightfieldMesh>(
                            Utils::GenerateBlankHeightMap(50, 50));
                        meshFilter.Meshes->Items.emplace_back(meshItem);
                        break;
                    }
                    case MeshPrimitiveType::Invalid:
                        break;
                }
            }

            meshFilter.AssignEntityID(static_cast<int>(entity));
        });
        m_Registry.view<TerrainComponent>().each([](entt::entity entity, TerrainComponent& terrain) {
            // TODO: Move to AssetManager
            terrain.Mesh = GeometryFactory::CreateMeshPrimitive<HeightfieldMesh>(
                terrain.TerrainHeightMap, terrain.XScale, terrain.YScale, terrain.ZScale);
        });

        // Material Loading
        m_Registry.view<MeshRendererComponent>().each([](entt::entity entity, MeshRendererComponent& renderer) {
            // TODO: Move to AssetManager
            if (FileSystem::Exists(renderer.MaterialFilePath))
            {
                renderer.Mat = IO::Load(renderer.MaterialFilePath);
            }
        });
        m_Registry.view<TerrainRendererComponent>().each([](entt::entity entity, TerrainRendererComponent& renderer) {
            // TODO: Move to AssetManager
            if (FileSystem::Exists(renderer.MaterialFilePath))
            {
                renderer.Mat = IO::Load(renderer.MaterialFilePath);
            }
        });
        m_Registry.view<UI::ImageComponent>().each([](entt::entity e, UI::ImageComponent& image){
            // TODO: Move to AssetManager
            if (FileSystem::Exists(image.MaterialFilePath))
            {
                image.Mat = IO::Load(image.MaterialFilePath);
            }
        });
        m_Registry.view<UI::ButtonComponent>().each([](entt::entity e, UI::ButtonComponent& button){
            // TODO: Move to AssetManager
            if (FileSystem::Exists(button.MaterialFilePath))
            {
                button.Mat = IO::Load(button.MaterialFilePath);
            }
        });

        // Init Animators
        m_Registry.view<AnimatorComponent>().each(
            [](entt::entity entity, AnimatorComponent& animator) { animator.Controller.Init(); });

        // Scripting Callback
        m_Registry.view<NativeScriptingComponent>().each([this](entt::entity              entity,
                                                                NativeScriptingComponent& nativeScript) {
            if (nativeScript.ScriptInstance == nullptr)
            {
                nativeScript.ScriptInstance = TypeFactory::CreateObject<NativeScriptInstance>(nativeScript.ScriptName);
                nativeScript.ScriptInstance->m_OwnerEntity = CreateRef<Entity>(entity, this);
            }
            nativeScript.ScriptInstance->OnLoad();
        });

        LogicSceneLoadedEvent loadedEvent(this);
        TriggerEvent(loadedEvent);
    }

    void LogicScene::OnTick(float deltaTime)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        // Tick NativeScriptingComponents for now
        // TODO: Consider Script Tick Priority
        // TODO: If time is enough, integrate Lua or C# Scripting.
        m_Registry.view<NativeScriptingComponent>().each(
            [this, deltaTime](entt::entity entity, NativeScriptingComponent& nativeScript) {
                if (nativeScript.ScriptInstance->GetEnabled())
                {
                    if (m_SimulationMode == LogicSceneSimulationMode::Editor &&
                        nativeScript.ScriptInstance->IsEditorScript())
                    {
                        // Tick editor scripts
                        nativeScript.ScriptInstance->OnTick(deltaTime);
                    }
                    else if (m_SimulationMode == LogicSceneSimulationMode::Game &&
                             m_SimulationStatus == LogicSceneSimulationStatus::Simulating &&
                             !nativeScript.ScriptInstance->IsEditorScript())
                    {
                        // Tick game scripts
                        nativeScript.ScriptInstance->OnTick(deltaTime);
                    }
                }
            });

        if (m_SimulationStatus != LogicSceneSimulationStatus::Simulating)
        {
            return;
        }

        // Built-in camera controllers
        m_Registry.view<TransformComponent, CameraComponent, FreeMoveCameraControllerComponent>().each(
            [](entt::entity                       entity,
               TransformComponent&                transform,
               CameraComponent&                   camera,
               FreeMoveCameraControllerComponent& freeMoveController) {
                auto& inputSystem   = g_EngineContext->InputSys;
                auto  mousePosition = inputSystem->GetMousePosition();

                auto offset = mousePosition - freeMoveController.LastFrameMousePosition;
                offset *= freeMoveController.Sensitivity;

                if (freeMoveController.IsFirstTime)
                {
                    offset                         = {0, 0};
                    freeMoveController.IsFirstTime = false;
                }

                auto cameraRotationEuler = transform.GetRotationEuler();

                float yaw   = cameraRotationEuler.y;
                float pitch = cameraRotationEuler.x;

                yaw += offset.x;
                pitch -= offset.y;

                if (pitch > 89.0f)
                {
                    pitch = 89.0f;
                }

                if (pitch < -89.0f)
                {
                    pitch = -89.0f;
                }

                cameraRotationEuler.y = yaw;
                cameraRotationEuler.x = pitch;

                transform.SetRotationEuler(cameraRotationEuler);
                freeMoveController.LastFrameMousePosition = mousePosition;

                // Calculate forward (Yaw - 90 to adjust)
                glm::vec3 forward;
                forward.x = cos(glm::radians(cameraRotationEuler.y - 90)) * cos(glm::radians(cameraRotationEuler.x));
                forward.y = sin(glm::radians(cameraRotationEuler.x));
                forward.z = sin(glm::radians(cameraRotationEuler.y - 90)) * cos(glm::radians(cameraRotationEuler.x));
                forward   = glm::normalize(forward);

                glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

                if (inputSystem->GetKey(KeyCode::W))
                {
                    transform.Position += forward * freeMoveController.Speed;
                }

                if (inputSystem->GetKey(KeyCode::S))
                {
                    transform.Position -= forward * freeMoveController.Speed;
                }

                if (inputSystem->GetKey(KeyCode::A))
                {
                    transform.Position -= right * freeMoveController.Speed;
                }

                if (inputSystem->GetKey(KeyCode::D))
                {
                    transform.Position += right * freeMoveController.Speed;
                }
            });

        // Animators
        m_Registry.view<AnimatorComponent>().each(
            [deltaTime](entt::entity entity, AnimatorComponent& animator) { animator.Controller.Update(deltaTime); });
    }

    void LogicScene::OnFixedTick()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        m_Registry.view<NativeScriptingComponent>().each([this](entt::entity              entity,
                                                                NativeScriptingComponent& nativeScript) {
            if (m_SimulationMode == LogicSceneSimulationMode::Editor && nativeScript.ScriptInstance->IsEditorScript())
            {
                // Fixed Tick editor scripts
                nativeScript.ScriptInstance->OnFixedTick();
            }
            else if (m_SimulationMode == LogicSceneSimulationMode::Game &&
                     m_SimulationStatus == LogicSceneSimulationStatus::Simulating &&
                     !nativeScript.ScriptInstance->IsEditorScript())
            {
                // Fixed Tick game scripts
                nativeScript.ScriptInstance->OnFixedTick();
            }
        });
    }

    void LogicScene::OnUnload()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        m_Registry.view<NativeScriptingComponent>().each(
            [](entt::entity entity, NativeScriptingComponent& nativeScript) {
                nativeScript.ScriptInstance->OnUnload();
            });

        LogicSceneUnloadedEvent unloadedEvent(this);
        TriggerEvent(unloadedEvent);
    }

    void LogicScene::SaveTo(const std::filesystem::path& dstPath)
    {
        m_Path = dstPath;
        IO::Serialize(this, dstPath);
    }

    void LogicScene::LoadFrom()
    {
        if (std::filesystem::exists(m_Path))
        {
            m_Registry.clear();
            IO::Deserialize(this, m_Path);
            InitAfterDeserializing();
        }
    }

    void LogicScene::LoadFrom(const std::filesystem::path& srcPath)
    {
        if (std::filesystem::exists(srcPath))
        {
            m_Registry.clear();
            m_Path = srcPath;
            IO::Deserialize(this, srcPath);
            InitAfterDeserializing();
        }
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

    void LogicScene::CreateDefaultEntities()
    {
        Entity directionalLight          = CreateEntity("Directional Light");
        auto&  directionalLightComponent = directionalLight.AddComponent<DirectionalLightComponent>();
        // FIXME:
        // directionalLightComponent.ShadowMaterial =
        //     DzMaterial::LoadFromPath("Assets/Materials/Legacy/ShadowMapping.dzmaterial");
    }

    std::string LogicScene::GetNameFromEntity(Entity entity) const
    {
        if (const auto* nameComponent = m_Registry.try_get<NameComponent>(entity))
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

    std::string LogicScene::ExtractEntityName(const std::string& name)
    {
        std::smatch match;

        if (std::regex_search(name, match, std::regex {R"((.*?)\s*\((\d+)\))"}))
        {
            return match[1].str();
        }

        return "";
    }

    void LogicScene::InitAfterDeserializing()
    {
        m_EntityMap->clear();
        m_Name2CountMap.clear();

        m_Registry.view<IDComponent, NameComponent>().each(
            [this](entt::entity entity, IDComponent& id, NameComponent& name) {
                (*m_EntityMap)[id.Id] = {entity, this};
                auto entityName       = ExtractEntityName(name.Name);
                if (m_Name2CountMap.count(entityName) == 0)
                    m_Name2CountMap[entityName] = 0;
                m_Name2CountMap[entityName]++;
            });
    }

    template<typename T>
    void LogicScene::OnComponentAdded(Entity entity, T& component)
    {}

    ON_COMPONENT_ADDED(IDComponent) {}
    ON_COMPONENT_ADDED(NameComponent) {}
    ON_COMPONENT_ADDED(TagComponent) {}
    ON_COMPONENT_ADDED(LayerComponent) {}
    ON_COMPONENT_ADDED(TreeNodeComponent) {}
    ON_COMPONENT_ADDED(TransformComponent) {}
    ON_COMPONENT_ADDED(EntityStatusComponent) {}

    ON_COMPONENT_ADDED(NativeScriptingComponent)
    {
        // Bind entity
        Ref<Entity> entityCopy = CreateRef<Entity>(entity);
        if (component.ScriptInstance == nullptr)
        {
            component.ScriptInstance = TypeFactory::CreateObject<NativeScriptInstance>(component.ScriptName);
        }
        component.ScriptInstance->m_OwnerEntity = entityCopy;
    }

    ON_COMPONENT_ADDED(RigidBodyComponent) {}
    ON_COMPONENT_ADDED(SphereColliderComponent) {}
    ON_COMPONENT_ADDED(BoxColliderComponent) {}
    ON_COMPONENT_ADDED(CapsuleColliderComponent) {}
    ON_COMPONENT_ADDED(TerrainColliderComponent) {}
    ON_COMPONENT_ADDED(CharacterControllerComponent) {}
    ON_COMPONENT_ADDED(MeshColliderComponent) {}

    ON_COMPONENT_ADDED(CameraComponent) {}
    ON_COMPONENT_ADDED(FreeMoveCameraControllerComponent) {}
    ON_COMPONENT_ADDED(DirectionalLightComponent) {}
    ON_COMPONENT_ADDED(PointLightComponent) {}
    ON_COMPONENT_ADDED(MeshFilterComponent) {}
    ON_COMPONENT_ADDED(MeshRendererComponent) {}
    ON_COMPONENT_ADDED(TerrainComponent) {}
    ON_COMPONENT_ADDED(TerrainRendererComponent) {}

    ON_COMPONENT_ADDED(AnimatorComponent) {}

    ON_COMPONENT_ADDED(UI::CanvasComponent) {}
    ON_COMPONENT_ADDED(UI::RectTransformComponent) {}
    ON_COMPONENT_ADDED(UI::ButtonComponent) {}
    ON_COMPONENT_ADDED(UI::ImageComponent) {}
    ON_COMPONENT_ADDED(UI::TextComponent) {}
} // namespace SnowLeopardEngine
