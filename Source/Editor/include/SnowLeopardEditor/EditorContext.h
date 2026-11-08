#pragma once

#include "SnowLeopardEditor/EditorCamera.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

#include <filesystem>
#include <functional>
#include <optional>

namespace SnowLeopardEngine
{
    struct Model;
}

namespace SnowLeopardEngine::Editor
{
    enum class EditorMode
    {
        Edit,
        Playing,
        Paused,
    };

    // What the editor is working on, shared by the panels: the edited scene and its file, play mode (a copy of the
    // scene is simulated and thrown away on Stop), the selection and the Viewport camera.
    class EditorContext
    {
    public:
        static void Init(const std::filesystem::path& projectFile);
        static void Shutdown();
        // Once per frame, before the panels: applies file-dialog results and deferred scene switches.
        static void OnTick();

        // ---- scenes
        static Ref<LogicScene>              GetEditingScene() { return s_EditingScene; }
        static Ref<LogicScene>              GetActiveScene(); // the simulated copy while playing
        static const std::filesystem::path& GetScenePath() { return s_ScenePath; }
        static std::string                  GetSceneTitle();

        static void NewScene();               // the default editing scene
        static bool OpenScene(const std::filesystem::path& path);
        static bool SaveScene();              // to its file, else asks for one
        static bool SaveSceneAs(const std::filesystem::path& path);
        // Native file dialogs (asynchronous: the result is applied by OnTick).
        static void ShowOpenSceneDialog();
        static void ShowSaveSceneDialog();
        // Double-clicked in the Resources panel.
        static void OpenAsset(const std::filesystem::path& path);

        // ---- play mode
        static EditorMode GetMode() { return s_Mode; }
        static void       Play();  // Edit -> Playing (copy of the scene), Paused -> Playing
        static void       Pause(); // Playing -> Paused
        static void       Stop();  // back to the editing scene

        // ---- selection (entity UUIDs: they are the same in the simulated copy)
        static std::vector<Entity> GetSelection();    // live entities of the active scene
        static Entity              GetPrimarySelection(); // the last selected
        static void                Select(Entity entity, bool additive = false);
        static void                Deselect(Entity entity);
        static void                ClearSelection();
        static bool                IsSelected(Entity entity);

        // ---- edits
        static Entity CreateEntity(const std::string& name, Entity parent = {}); // selected
        static Entity CreatePrimitive(MeshPrimitiveType type, const std::string& name, Entity parent = {});
        static Entity CreateLight(bool directional, Entity parent = {});
        static Entity CreateCamera(Entity parent = {});
        static void   DuplicateSelection();
        static void   DeleteSelection();

        static EditorCamera& GetCamera() { return s_Camera; }
        static void          FocusSelection();

        // Panel visibility (View menu)
        struct PanelVisibility
        {
            bool Hierarchy      = true;
            bool Inspector      = true;
            bool Resources      = true;
            bool Console        = true;
            bool RenderSettings = true;
            bool ImGuiDemo      = false;
        };
        static PanelVisibility& GetPanels() { return s_Panels; }

    private:
        static void ActivateEditingScene(const Ref<LogicScene>& scene, const std::filesystem::path& path);
        static void BuildDefaultScene(LogicScene& scene);

        static Ref<LogicScene>       s_EditingScene;
        static Ref<LogicScene>       s_SimulatingScene;
        static std::filesystem::path s_ScenePath;
        static std::filesystem::path s_ProjectDirectory;
        static EditorMode            s_Mode;
        static EditorCamera          s_Camera;
        static PanelVisibility       s_Panels;
        static Ref<Model>            s_DefaultCharacter; // the default scene's animated model (kept alive with it)
    };
} // namespace SnowLeopardEngine::Editor
