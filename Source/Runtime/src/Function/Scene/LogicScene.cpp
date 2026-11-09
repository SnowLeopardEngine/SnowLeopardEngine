#include "SnowLeopardEngine/Function/Scene/LogicScene.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Animation/Animator.h"
#include "SnowLeopardEngine/Function/Asset/AssetManager.h"
#include "SnowLeopardEngine/Function/Asset/Model.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/IO/AssimpModelLoader.h"
#include "SnowLeopardEngine/Function/IO/MaterialLoader.h"
#include "SnowLeopardEngine/Function/IO/Serialization.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/matrix_decompose.hpp>

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

        // Create entities in new scene, in the source's creation order (views iterate newest first), so the copy
        // lists them in the same order (editor play mode).
        auto                      idView = srcSceneRegistry.view<IDComponent>();
        std::vector<entt::entity> ordered(idView.begin(), idView.end());
        std::sort(ordered.begin(), ordered.end(), [](entt::entity a, entt::entity b) {
            return entt::to_entity(a) < entt::to_entity(b);
        });
        for (auto e : ordered)
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
        // Destroy children (a copy of the list: each child removes itself from it)
        if (entity.HasChildren())
        {
            const auto children = entity.GetChildrenUUIDs();
            for (const auto& childUUID : children)
            {
                if (auto child = GetEntityWithCoreUUID(childUUID))
                {
                    DestroyEntity(child);
                }
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

    glm::mat4 LogicScene::GetParentWorldTransform(entt::entity entity) const
    {
        const auto* node = m_Registry.try_get<TreeNodeComponent>(entity);
        if (node == nullptr || node->ParentId.is_nil())
        {
            return glm::mat4(1);
        }
        const auto found = m_EntityMap->find(node->ParentId);
        if (found == m_EntityMap->end() || !m_Registry.valid(entt::entity(found->second)))
        {
            return glm::mat4(1);
        }
        return GetWorldTransform(entt::entity(found->second));
    }

    glm::mat4 LogicScene::GetWorldTransform(entt::entity entity) const
    {
        const auto* transform = m_Registry.try_get<TransformComponent>(entity);
        const auto  local     = transform != nullptr ? transform->GetTransform() : glm::mat4(1);
        return GetParentWorldTransform(entity) * local;
    }

    bool LogicScene::IsDescendantOf(Entity entity, Entity ancestor) const
    {
        auto current = entity;
        for (int depth = 0; current && depth < 1024; ++depth)
        {
            if (current == ancestor)
            {
                return true;
            }
            if (!current.HasParent())
            {
                return false;
            }
            current = GetEntityWithCoreUUID(current.GetParentUUID());
        }
        return false;
    }

    bool LogicScene::SetParent(Entity child, Entity parent, bool keepWorld)
    {
        if (!child || (parent && IsDescendantOf(parent, child)))
        {
            return false;
        }
        const auto world = GetWorldTransform(child);
        if (child.HasParent())
        {
            if (auto previous = GetEntityWithCoreUUID(child.GetParentUUID()))
            {
                previous.RemoveChild(child.GetCoreUUID());
            }
        }
        auto& node    = child.GetComponent<TreeNodeComponent>();
        node.ParentId = parent ? parent.GetCoreUUID() : CoreUUID {};
        if (parent)
        {
            parent.AddChild(child.GetCoreUUID());
        }
        if (keepWorld)
        {
            const auto local = glm::inverse(GetParentWorldTransform(child)) * world;
            glm::vec3  position, scale, skew;
            glm::quat  rotation;
            glm::vec4  perspective;
            if (glm::decompose(local, scale, rotation, position, skew, perspective))
            {
                auto& transform    = child.GetComponent<TransformComponent>();
                transform.Position = position;
                transform.Scale    = scale;
                transform.SetRotationEuler(glm::degrees(glm::eulerAngles(rotation)));
            }
        }
        return true;
    }

    std::vector<Entity> LogicScene::GetRootEntities()
    {
        std::vector<Entity> roots;
        for (auto entity : m_Registry.view<IDComponent, TreeNodeComponent>())
        {
            const auto& node = m_Registry.get<TreeNodeComponent>(entity);
            if (node.ParentId.is_nil() || !GetEntityWithCoreUUID(node.ParentId))
            {
                roots.emplace_back(entity, this);
            }
        }
        // Creation order (entity index; a recycled index takes a destroyed entity's place).
        std::sort(roots.begin(), roots.end(), [](Entity a, Entity b) {
            return entt::to_entity(entt::entity(a)) < entt::to_entity(entt::entity(b));
        });
        return roots;
    }

    void LogicScene::TriggerEntityDestroy(Entity entity)
    {
        if (m_IsLoaded)
        {
            EntityDestroyEvent entityDestroyEvent(entity);
            TriggerEvent(entityDestroyEvent);
        }
    }

    void LogicScene::TriggerEntityCreate(Entity entity)
    {
        if (m_IsLoaded)
        {
            // Components are added after CreateEntity returns: their meshes and materials are loaded by
            // LoadPendingRenderResources at the start of the next tick.
            EntityCreateEvent entityCreateEvent(entity);
            TriggerEvent(entityCreateEvent);
        }
    }

    void LogicScene::LoadEntityRenderResources(entt::entity entity)
    {
        // TODO: Move to AssetManager
        // Idempotent: only what is still missing is built or loaded.
        if (auto* meshFilter = m_Registry.try_get<MeshFilterComponent>(entity); meshFilter != nullptr && meshFilter->Meshes == nullptr)
        {
            if (meshFilter->PrimitiveType != MeshPrimitiveType::Invalid)
            {
                meshFilter->Meshes = CreateRef<MeshGroup>();

                switch (meshFilter->PrimitiveType)
                {
                    case MeshPrimitiveType::Quad:
                        meshFilter->Meshes->Items.emplace_back(GeometryFactory::CreateMeshPrimitive<QuadMesh>());
                        break;
                    case MeshPrimitiveType::Cube:
                        meshFilter->Meshes->Items.emplace_back(GeometryFactory::CreateMeshPrimitive<CubeMesh>());
                        break;
                    case MeshPrimitiveType::Sphere:
                        meshFilter->Meshes->Items.emplace_back(GeometryFactory::CreateMeshPrimitive<SphereMesh>());
                        break;
                    case MeshPrimitiveType::Capsule:
                        meshFilter->Meshes->Items.emplace_back(GeometryFactory::CreateMeshPrimitive<CapsuleMesh>());
                        break;
                    case MeshPrimitiveType::Heightfield:
                        meshFilter->Meshes->Items.emplace_back(
                            GeometryFactory::CreateMeshPrimitive<HeightfieldMesh>(Utils::GenerateBlankHeightMap(50, 50)));
                        break;
                    case MeshPrimitiveType::Invalid:
                        break;
                }
            }
            else if (const auto id = AssetManager::FindBySource(meshFilter->FilePath); id && (meshFilter->Meshes = AssetManager::LoadMesh(*id)))
            {
                // the imported mesh (shared by every entity that uses the file)
            }
            else if (FileSystem::Exists(meshFilter->FilePath))
            {
                AssetManager::ReportFallback(meshFilter->FilePath);
                meshFilter->Meshes = CreateRef<MeshGroup>();
                Model assimpModel;
                AssimpModelLoader::LoadModel(meshFilter->FilePath, assimpModel);
                *meshFilter->Meshes = assimpModel.Meshes;
            }
        }

        if (auto* terrain = m_Registry.try_get<TerrainComponent>(entity); terrain != nullptr && terrain->Mesh.Data.Vertices.empty())
        {
            terrain->Mesh = GeometryFactory::CreateMeshPrimitive<HeightfieldMesh>(
                terrain->TerrainHeightMap, terrain->XScale, terrain->YScale, terrain->ZScale);
        }

        auto loadMaterial = [](auto* renderer) {
            if (renderer != nullptr && renderer->Mat == nullptr && FileSystem::Exists(renderer->MaterialFilePath))
            {
                renderer->Mat = IO::Load(renderer->MaterialFilePath);
            }
        };
        loadMaterial(m_Registry.try_get<MeshRendererComponent>(entity));
        loadMaterial(m_Registry.try_get<TerrainRendererComponent>(entity));
        loadMaterial(m_Registry.try_get<UI::ImageComponent>(entity));
        loadMaterial(m_Registry.try_get<UI::ButtonComponent>(entity));
        loadMaterial(m_Registry.try_get<UI::TextComponent>(entity));
    }

    void LogicScene::LoadPendingRenderResources()
    {
        std::vector<entt::entity> pending;
        m_Registry.view<MeshFilterComponent>().each([&](entt::entity entity, MeshFilterComponent& filter) {
            if (filter.Meshes == nullptr && (filter.PrimitiveType != MeshPrimitiveType::Invalid || !filter.FilePath.empty()))
            {
                pending.push_back(entity);
            }
        });
        m_Registry.view<TerrainComponent>().each([&](entt::entity entity, TerrainComponent& terrain) {
            if (terrain.Mesh.Data.Vertices.empty())
            {
                pending.push_back(entity);
            }
        });
        auto missingMaterial = [&](auto& view) {
            view.each([&](entt::entity entity, auto& renderer) {
                if (renderer.Mat == nullptr && !renderer.MaterialFilePath.empty())
                {
                    pending.push_back(entity);
                }
            });
        };
        auto meshRenderers    = m_Registry.view<MeshRendererComponent>();
        auto terrainRenderers = m_Registry.view<TerrainRendererComponent>();
        auto images           = m_Registry.view<UI::ImageComponent>();
        auto buttons          = m_Registry.view<UI::ButtonComponent>();
        auto texts            = m_Registry.view<UI::TextComponent>();
        missingMaterial(meshRenderers);
        missingMaterial(terrainRenderers);
        missingMaterial(images);
        missingMaterial(buttons);
        missingMaterial(texts);
        std::sort(pending.begin(), pending.end());
        pending.erase(std::unique(pending.begin(), pending.end()), pending.end());
        for (auto entity : pending)
        {
            LoadEntityRenderResources(entity);
        }
    }

    Entity LogicScene::GetEntityWithCoreUUID(CoreUUID id) const
    {
        // A null Entity for an unknown id (a stale selection, a dangling parent id), instead of throwing.
        const auto found = m_EntityMap->find(id);
        return found != m_EntityMap->end() ? found->second : Entity {};
    }

    Entity LogicScene::DuplicateEntity(Entity source)
    {
        if (!source)
        {
            return {};
        }
        Entity copy = CreateEntity(source.GetName());
        CopyComponentIfExists(AllCopyableComponents {}, copy, source);

        // The copy is a sibling of the source; its children are copies of the source's children.
        auto& node = copy.GetComponent<TreeNodeComponent>();
        node.ChildrenIds.clear();
        if (auto parent = GetEntityWithCoreUUID(node.ParentId))
        {
            parent.AddChild(copy.GetCoreUUID());
        }
        else
        {
            node.ParentId = {};
        }
        const auto children = source.GetChildrenUUIDs();
        for (const auto& childId : children)
        {
            if (auto childCopy = DuplicateEntity(GetEntityWithCoreUUID(childId)))
            {
                SetParent(childCopy, copy, false);
            }
        }
        return copy;
    }

    Entity LogicScene::GetEntityWithName(const std::string& name)
    {
        auto view = m_Registry.view<NameComponent>();
        for (auto entity : view)
        {
            auto& nameComponent = view.get<NameComponent>(entity);
            if (nameComponent.Name == name)
            {
                return {entity, this};
            }
        }

        return {};
    }

    void LogicScene::OnLoad()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        LogicScenePreLoadEvent preloadEvent(this);
        TriggerEvent(preloadEvent);

        // Meshes and materials (dirty code for now; TODO: Move to AssetManager)
        for (auto entity : m_Registry.view<entt::entity>())
        {
            LoadEntityRenderResources(entity);
        }

        // Load Audio
        m_Registry.view<TransformComponent, AudioListenerComponent>().each(
            [](entt::entity entity, TransformComponent& transform, AudioListenerComponent& audioListener) {
                g_EngineContext->AudioSys->SetListenerCone(audioListener.ConeInfo.ConeInnerAngleRadians,
                                                           audioListener.ConeInfo.ConeOuterAngleRadians,
                                                           audioListener.ConeInfo.ConeOuterGain);
            });
        m_Registry.view<AudioSourceComponent>().each([](entt::entity entity, AudioSourceComponent& audioSource) {
            if (FileSystem::Exists(audioSource.AudioPath))
            {
                if (audioSource.Clip == nullptr)
                {
                    audioSource.Clip =
                        g_EngineContext->AudioSys->CreateAudioClip(audioSource.AudioPath.generic_string());
                }

                audioSource.Clip->SetIsLoop(audioSource.IsLoop);
                audioSource.Clip->SetIsSpatial(audioSource.IsSpatial);
                audioSource.Clip->SetVolume(audioSource.Volume);
                audioSource.Clip->SetDistanceModel(audioSource.DistanceModel);
                audioSource.Clip->SetCone(audioSource.ConeInfo.ConeInnerAngleRadians,
                                          audioSource.ConeInfo.ConeOuterAngleRadians,
                                          audioSource.ConeInfo.ConeOuterGain);
                audioSource.Clip->SetRollOff(audioSource.RollOff);
                audioSource.Clip->SetGainMinMax(audioSource.MinMaxGain);

                if (audioSource.PlayOnAwake)
                {
                    audioSource.Clip->Play();
                }
            }
        });

        // Init Animators
        m_Registry.view<AnimatorComponent>().each(
            [](entt::entity entity, AnimatorComponent& animator) { animator.CurrentAnimator.InitAnimators(); });

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

        m_IsLoaded = true;
    }

    void LogicScene::OnTick(float deltaTime)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        // Entities created (or given renderers) since the last tick.
        LoadPendingRenderResources();

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

        // Built-in camera controllers
        m_Registry.view<TransformComponent, CameraComponent, ThirdPersonFollowCameraControllerComponent>().each(
            [this](entt::entity                                entity,
                   TransformComponent&                         transform,
                   CameraComponent&                            camera,
                   ThirdPersonFollowCameraControllerComponent& thirdPersonController) {
                auto& inputSystem   = g_EngineContext->InputSys;
                auto  mousePosition = inputSystem->GetMousePosition();

                auto targetTransform = m_Registry.get<TransformComponent>(thirdPersonController.FollowEntity);
                // auto targetRotationEuler = targetTransform.GetRotationEuler();
                // // Calculate forward (Yaw - 90 to adjust)
                // glm::vec3 forward;
                // forward.x = cos(glm::radians(targetRotationEuler.y - 90)) * cos(glm::radians(targetRotationEuler.x));
                // forward.y = sin(glm::radians(targetRotationEuler.x));
                // forward.z = sin(glm::radians(targetRotationEuler.y - 90)) * cos(glm::radians(targetRotationEuler.x));
                // forward   = glm::normalize(forward);
                // auto up   = glm::vec3(0, 1, 0);

                transform.Position = targetTransform.Position + thirdPersonController.Offset;

                // transform.SetRotationEuler(
                //     glm::vec3(targetRotationEuler.x, targetRotationEuler.y - 180, targetRotationEuler.z));
            });

        // Audio
        m_Registry.view<TransformComponent, AudioListenerComponent>().each(
            [](entt::entity entity, TransformComponent& transform, AudioListenerComponent& audioListener) {
                g_EngineContext->AudioSys->SetListenerPosition(transform.Position);
                g_EngineContext->AudioSys->SetListenerDirection(transform.GetRotationEuler());
            });
        m_Registry.view<TransformComponent, AudioSourceComponent>().each(
            [](entt::entity entity, TransformComponent& transform, AudioSourceComponent& audioSource) {
                audioSource.Clip->SetPosition(transform.Position);
                auto worldDirection = transform.GetRotation() * audioSource.LocalDirection;
                audioSource.Clip->SetDirection(worldDirection);
            });

        // Animators
        m_Registry.view<AnimatorComponent>().each(
            [deltaTime](entt::entity entity, AnimatorComponent& animator) { animator.CurrentAnimator.UpdateAnimator(deltaTime); });
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

        m_Registry.view<TransformComponent, AudioListenerComponent>().each(
            [](entt::entity entity, TransformComponent& transform, AudioListenerComponent& audioListener) {
                g_EngineContext->AudioSys->SetListenerPosition({0, 0, 0});
                g_EngineContext->AudioSys->SetListenerDirection({0, 0, 0});
            });
        m_Registry.view<TransformComponent, AudioSourceComponent>().each(
            [](entt::entity entity, TransformComponent& transform, AudioSourceComponent& audioSource) {
                audioSource.Clip->Stop();
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
    ON_COMPONENT_ADDED(ThirdPersonFollowCameraControllerComponent) {}
    ON_COMPONENT_ADDED(DirectionalLightComponent) {}
    ON_COMPONENT_ADDED(PointLightComponent) {}
    ON_COMPONENT_ADDED(MeshFilterComponent) {}
    ON_COMPONENT_ADDED(MeshRendererComponent) {}
    ON_COMPONENT_ADDED(TerrainComponent) {}
    ON_COMPONENT_ADDED(TerrainRendererComponent) {}

    ON_COMPONENT_ADDED(AnimatorComponent) {}

    ON_COMPONENT_ADDED(AudioSourceComponent) {}
    ON_COMPONENT_ADDED(AudioListenerComponent) {}

    ON_COMPONENT_ADDED(UI::CanvasComponent) {}
    ON_COMPONENT_ADDED(UI::RectTransformComponent) {}
    ON_COMPONENT_ADDED(UI::ButtonComponent) {}
    ON_COMPONENT_ADDED(UI::ImageComponent) {}
    ON_COMPONENT_ADDED(UI::TextComponent) {}
} // namespace SnowLeopardEngine
