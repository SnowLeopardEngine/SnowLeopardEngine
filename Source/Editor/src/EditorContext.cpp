#include "SnowLeopardEditor/EditorContext.h"
#include "SnowLeopardEditor/Selector.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Animation/AnimatorController.h"
#include "SnowLeopardEngine/Function/Asset/Model.h"
#include "SnowLeopardEngine/Function/IO/OzzModelLoader.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

#include <SDL3/SDL.h>

#include <mutex>

namespace SnowLeopardEngine::Editor
{
    Ref<LogicScene>                EditorContext::s_EditingScene    = nullptr;
    Ref<LogicScene>                EditorContext::s_SimulatingScene = nullptr;
    std::filesystem::path          EditorContext::s_ScenePath;
    std::filesystem::path          EditorContext::s_ProjectDirectory;
    EditorMode                     EditorContext::s_Mode = EditorMode::Edit;
    EditorCamera                   EditorContext::s_Camera;
    EditorContext::PanelVisibility EditorContext::s_Panels;
    Ref<Model>                     EditorContext::s_DefaultCharacter;

    namespace
    {
        constexpr SelectionCategory kScene = SelectionCategory::Viewport;

        const SDL_DialogFileFilter kSceneFilter[] {{"Scene (*.dzscene)", "dzscene"}};

        // File-dialog results: the callback may run on a dialog thread, so it only queues them for OnTick.
        std::mutex                                          g_DialogMutex;
        std::vector<std::pair<bool, std::filesystem::path>> g_DialogResults; // (save, path)

        void SDLCALL OnDialog(void* userdata, const char* const* files, int)
        {
            if (files == nullptr || files[0] == nullptr)
            {
                return; // cancelled, or failed (SDL_GetError)
            }
            const bool            save = userdata != nullptr;
            std::filesystem::path path = std::filesystem::u8path(files[0]);
            if (save && path.extension() != ".dzscene")
            {
                path += ".dzscene";
            }
            std::lock_guard lock(g_DialogMutex);
            g_DialogResults.emplace_back(save, path);
        }

        SDL_Window* NativeWindow()
        {
            return static_cast<SDL_Window*>(g_EngineContext->WindowSys->GetWindow()->GetPlatformWindow());
        }
    } // namespace

    // ------------------------------------------------------------------------------------------------ lifetime

    void EditorContext::Init(const std::filesystem::path& projectFile)
    {
        s_ProjectDirectory = projectFile.parent_path();
        NewScene();

        // SLE_EDITOR_SELECT=<entity name>: start with that entity selected (offline captures of the gizmo / outline,
        // which have no mouse).
        if (const char* name = std::getenv("SLE_EDITOR_SELECT"); name != nullptr && *name != 0 && s_EditingScene != nullptr)
        {
            Select(s_EditingScene->GetEntityWithName(name));
        }
        // SLE_EDITOR_PLAY=1: start in play mode (offline check of Play: physics and scripts on the scene copy).
        if (const char* play = std::getenv("SLE_EDITOR_PLAY"); play != nullptr && play[0] == '1')
        {
            Play();
        }
    }

    void EditorContext::Shutdown()
    {
        if (s_Mode != EditorMode::Edit)
        {
            Stop();
        }
        g_EngineContext->RenderSys->ClearCameraOverride();
        Selector::UnselectAll();
        s_SimulatingScene.reset();
        s_EditingScene.reset();
        g_EngineContext->SceneMngr->SetActiveScene(nullptr);
        s_DefaultCharacter.reset(); // its meshes own GPU data: released while the engine is up
    }

    void EditorContext::OnTick()
    {
        std::vector<std::pair<bool, std::filesystem::path>> results;
        {
            std::lock_guard lock(g_DialogMutex);
            results.swap(g_DialogResults);
        }
        for (const auto& [save, path] : results)
        {
            save ? SaveSceneAs(path) : OpenScene(path);
        }

        // While editing, the Viewport draws through the editor camera; playing uses the scene's own camera.
        if (s_Mode == EditorMode::Edit)
        {
            g_EngineContext->RenderSys->SetCameraOverride(s_Camera.GetOverride());
        }
        else
        {
            g_EngineContext->RenderSys->ClearCameraOverride();
        }
    }

    // ------------------------------------------------------------------------------------------------ scenes

    Ref<LogicScene> EditorContext::GetActiveScene() { return s_SimulatingScene ? s_SimulatingScene : s_EditingScene; }

    std::string EditorContext::GetSceneTitle()
    {
        return s_ScenePath.empty() ? std::string("Untitled Scene") : s_ScenePath.filename().generic_string();
    }

    void EditorContext::ActivateEditingScene(const Ref<LogicScene>& scene, const std::filesystem::path& path)
    {
        if (s_Mode != EditorMode::Edit)
        {
            Stop();
        }
        ClearSelection();
        scene->SetSimulationMode(LogicSceneSimulationMode::Editor);
        s_EditingScene = scene;
        s_ScenePath    = path;
        g_EngineContext->SceneMngr->SetActiveScene(scene);
    }

    void EditorContext::NewScene()
    {
        // A new scene comes with a directional light (LogicScene default entities).
        auto scene = CreateRef<LogicScene>("Untitled Scene");
        BuildDefaultScene(*scene);
        ActivateEditingScene(scene, {});
    }

    bool EditorContext::OpenScene(const std::filesystem::path& path)
    {
        if (!std::filesystem::exists(path))
        {
            SNOW_LEOPARD_ERROR("[Editor] Scene file doesn't exist: {0}", path.generic_string());
            return false;
        }
        auto scene = CreateRef<LogicScene>(path.stem().generic_string(), true);
        scene->LoadFrom(path);
        ActivateEditingScene(scene, path);
        SNOW_LEOPARD_INFO("[Editor] Opened scene {0}", path.generic_string());
        return true;
    }

    bool EditorContext::SaveScene()
    {
        if (s_ScenePath.empty())
        {
            ShowSaveSceneDialog();
            return false;
        }
        return SaveSceneAs(s_ScenePath);
    }

    bool EditorContext::SaveSceneAs(const std::filesystem::path& path)
    {
        if (s_EditingScene == nullptr)
        {
            return false;
        }
        // Always the editing scene: what a simulation did to its copy is not saved.
        s_EditingScene->SaveTo(path);
        s_ScenePath = path;
        SNOW_LEOPARD_INFO("[Editor] Saved scene to {0}", path.generic_string());
        return true;
    }

    void EditorContext::ShowOpenSceneDialog()
    {
        const auto location = (s_ProjectDirectory / "Assets").generic_string();
        SDL_ShowOpenFileDialog(OnDialog, nullptr, NativeWindow(), kSceneFilter, 1, location.c_str(), false);
    }

    void EditorContext::ShowSaveSceneDialog()
    {
        static int saveTag   = 1; // non-null userdata: a save dialog
        const auto location  = (s_ScenePath.empty() ? s_ProjectDirectory / "Assets" / "Untitled.dzscene" : s_ScenePath).generic_string();
        SDL_ShowSaveFileDialog(OnDialog, &saveTag, NativeWindow(), kSceneFilter, 1, location.c_str());
    }

    void EditorContext::OpenAsset(const std::filesystem::path& path)
    {
        if (path.extension() == ".dzscene")
        {
            OpenScene(path);
        }
        else
        {
            SNOW_LEOPARD_INFO("[Editor] No editor for {0} yet", path.generic_string());
        }
    }

    void EditorContext::BuildDefaultScene(LogicScene& scene)
    {
        Entity camera                                                = scene.CreateEntity("Main Camera");
        camera.GetComponent<TransformComponent>().Position          = {0, 8, 25};
        camera.GetComponent<TransformComponent>().SetRotationEuler({-15, 0, 0});
        camera.AddComponent<CameraComponent>().ClearFlags            = CameraClearFlags::Skybox;

        auto normalMaterial = CreateRef<PhysicsMaterial>(0.4, 0.4, 0.4);

        Entity floor                                         = scene.CreateEntity("Floor");
        floor.GetComponent<TransformComponent>().Scale       = {50, 1, 50};
        floor.GetComponent<EntityStatusComponent>().IsStatic = true;
        floor.AddComponent<RigidBodyComponent>();
        floor.AddComponent<BoxColliderComponent>(normalMaterial);
        floor.AddComponent<MeshFilterComponent>().PrimitiveType      = MeshPrimitiveType::Cube;
        floor.AddComponent<MeshRendererComponent>().MaterialFilePath = "Assets/Materials/Next/White.dzmaterial";

        Entity sphere                                      = scene.CreateEntity("Sphere");
        sphere.GetComponent<TransformComponent>().Position = {5, 6, 0};
        sphere.GetComponent<TransformComponent>().Scale    = {3, 3, 3};
        sphere.AddComponent<RigidBodyComponent>();
        sphere.AddComponent<SphereColliderComponent>(normalMaterial);
        sphere.AddComponent<MeshFilterComponent>().PrimitiveType      = MeshPrimitiveType::Sphere;
        sphere.AddComponent<MeshRendererComponent>().MaterialFilePath = "Assets/Materials/Next/Blue.dzmaterial";

        Entity cube                                      = scene.CreateEntity("Cube");
        cube.GetComponent<TransformComponent>().Position = {-6, 1.5, 2};
        cube.GetComponent<TransformComponent>().Scale    = {2, 2, 2};
        cube.AddComponent<MeshFilterComponent>().PrimitiveType      = MeshPrimitiveType::Cube;
        cube.AddComponent<MeshRendererComponent>().MaterialFilePath = "Assets/Materials/Next/Red.dzmaterial";

        // The animated character of the original editor scene. Its meshes come from an ozz model loaded here, which a
        // .dzscene cannot reference yet (Phase 4 asset pipeline): a saved copy of this scene has no character mesh.
        if (s_DefaultCharacter == nullptr)
        {
            OzzModelLoadConfig config = {};
            config.OzzMeshPath        = "Assets/Models/Vampire/mesh.ozz";
            config.OzzSkeletonPath    = "Assets/Models/Vampire/skeleton.ozz";
            config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/animation_Dancing.ozz");
            auto model = CreateRef<Model>();
            if (OzzModelLoader::Load(config, model.get()) && !model->AnimationClips.empty())
            {
                s_DefaultCharacter = model;
            }
        }
        if (s_DefaultCharacter != nullptr)
        {
            Entity character                                      = scene.CreateEntity("Character");
            character.GetComponent<TransformComponent>().Position = {0, 0.5f, 0};
            character.GetComponent<TransformComponent>().Scale    = {8, 8, 8};
            character.AddComponent<MeshFilterComponent>().Meshes  = ShareMeshes(s_DefaultCharacter);
            character.AddComponent<MeshRendererComponent>().MaterialFilePath = "Assets/Materials/Next/Vampire.dzmaterial";
            auto controller = CreateRef<AnimatorController>();
            controller->RegisterAnimationClip(s_DefaultCharacter->AnimationClips[0]);
            controller->SetEntryAnimationClip(s_DefaultCharacter->AnimationClips[0]);
            character.AddComponent<AnimatorComponent>().CurrentAnimator.SetController(controller);
        }
    }

    // ------------------------------------------------------------------------------------------------ play mode

    void EditorContext::Play()
    {
        if (s_Mode == EditorMode::Paused && s_SimulatingScene != nullptr)
        {
            s_SimulatingScene->SetSimulationStatus(LogicSceneSimulationStatus::Simulating);
            s_Mode = EditorMode::Playing;
            return;
        }
        if (s_Mode != EditorMode::Edit || s_EditingScene == nullptr)
        {
            return;
        }
        s_SimulatingScene = LogicScene::Copy(s_EditingScene);
        s_SimulatingScene->SetSimulationStatus(LogicSceneSimulationStatus::Simulating);
        g_EngineContext->SceneMngr->SetActiveScene(s_SimulatingScene);
        s_Mode = EditorMode::Playing;
    }

    void EditorContext::Pause()
    {
        if (s_Mode == EditorMode::Playing && s_SimulatingScene != nullptr)
        {
            s_SimulatingScene->SetSimulationStatus(LogicSceneSimulationStatus::Paused);
            s_Mode = EditorMode::Paused;
        }
    }

    void EditorContext::Stop()
    {
        if (s_SimulatingScene != nullptr)
        {
            s_SimulatingScene->SetSimulationStatus(LogicSceneSimulationStatus::Stopped);
        }
        s_Mode = EditorMode::Edit;
        if (s_EditingScene != nullptr)
        {
            g_EngineContext->SceneMngr->SetActiveScene(s_EditingScene);
        }
        s_SimulatingScene.reset();
    }

    // ------------------------------------------------------------------------------------------------ selection

    std::vector<Entity> EditorContext::GetSelection()
    {
        std::vector<Entity> entities;
        auto                scene = GetActiveScene();
        if (scene == nullptr)
        {
            return entities;
        }
        for (const auto& id : Selector::GetSelections(kScene))
        {
            if (auto entity = scene->GetEntityWithCoreUUID(id))
            {
                entities.push_back(entity);
            }
        }
        return entities;
    }

    Entity EditorContext::GetPrimarySelection()
    {
        auto scene = GetActiveScene();
        auto id    = Selector::GetLastSelection(kScene);
        return scene != nullptr && id.has_value() ? scene->GetEntityWithCoreUUID(*id) : Entity {};
    }

    void EditorContext::Select(Entity entity, bool additive)
    {
        if (!additive)
        {
            Selector::UnselectAll(kScene);
        }
        if (entity)
        {
            const auto id = entity.GetCoreUUID();
            Selector::Unselect(kScene, id); // re-selecting makes it the primary one
            Selector::Select(kScene, id);
        }
    }

    void EditorContext::Deselect(Entity entity)
    {
        if (entity)
        {
            Selector::Unselect(kScene, entity.GetCoreUUID());
        }
    }

    void EditorContext::ClearSelection() { Selector::UnselectAll(kScene); }

    bool EditorContext::IsSelected(Entity entity) { return entity && Selector::IsSelected(kScene, entity.GetCoreUUID()); }

    // ------------------------------------------------------------------------------------------------ edits

    Entity EditorContext::CreateEntity(const std::string& name, Entity parent)
    {
        auto scene = GetActiveScene();
        if (scene == nullptr)
        {
            return {};
        }
        Entity entity = scene->CreateEntity(name);
        if (parent)
        {
            scene->SetParent(entity, parent, false);
        }
        Select(entity);
        return entity;
    }

    Entity EditorContext::CreatePrimitive(MeshPrimitiveType type, const std::string& name, Entity parent)
    {
        Entity entity = CreateEntity(name, parent);
        if (entity)
        {
            // Mesh and material are loaded by the scene's next tick (LoadPendingRenderResources).
            entity.AddComponent<MeshFilterComponent>().PrimitiveType      = type;
            entity.AddComponent<MeshRendererComponent>().MaterialFilePath = "Assets/Materials/Next/White.dzmaterial";
        }
        return entity;
    }

    Entity EditorContext::CreateLight(bool directional, Entity parent)
    {
        Entity entity = CreateEntity(directional ? "Directional Light" : "Point Light", parent);
        if (entity)
        {
            if (directional)
                entity.AddComponent<DirectionalLightComponent>();
            else
                entity.AddComponent<PointLightComponent>();
        }
        return entity;
    }

    Entity EditorContext::CreateCamera(Entity parent)
    {
        Entity entity = CreateEntity("Camera", parent);
        if (entity)
        {
            entity.AddComponent<CameraComponent>().ClearFlags = CameraClearFlags::Skybox;
        }
        return entity;
    }

    void EditorContext::DuplicateSelection()
    {
        auto scene = GetActiveScene();
        if (scene == nullptr)
        {
            return;
        }
        std::vector<Entity> copies;
        for (auto entity : GetSelection())
        {
            // A selected descendant is copied with its selected ancestor.
            bool underSelected = false;
            for (auto other : GetSelection())
            {
                underSelected |= other != entity && scene->IsDescendantOf(entity, other);
            }
            if (!underSelected)
            {
                copies.push_back(scene->DuplicateEntity(entity));
            }
        }
        ClearSelection();
        for (auto copy : copies)
        {
            Select(copy, true);
        }
    }

    void EditorContext::DeleteSelection()
    {
        auto scene = GetActiveScene();
        if (scene == nullptr)
        {
            return;
        }
        // UUIDs, not entities: deleting a parent deletes its selected children too.
        const auto ids = Selector::GetSelections(kScene);
        ClearSelection();
        for (const auto& id : ids)
        {
            if (auto entity = scene->GetEntityWithCoreUUID(id))
            {
                scene->DestroyEntity(entity);
            }
        }
    }

    void EditorContext::FocusSelection()
    {
        auto scene = GetActiveScene();
        auto selection = GetSelection();
        if (scene == nullptr || selection.empty())
        {
            return;
        }
        glm::vec3 low(std::numeric_limits<float>::max()), high(-std::numeric_limits<float>::max());
        for (auto entity : selection)
        {
            const auto world  = scene->GetWorldTransform(entity);
            const auto center = glm::vec3(world[3]);
            const float size  = std::max({glm::length(glm::vec3(world[0])), glm::length(glm::vec3(world[1])), glm::length(glm::vec3(world[2]))});
            low               = glm::min(low, center - size * 0.5f);
            high              = glm::max(high, center + size * 0.5f);
        }
        s_Camera.Focus((low + high) * 0.5f, glm::length(high - low) * 0.5f);
    }
} // namespace SnowLeopardEngine::Editor
