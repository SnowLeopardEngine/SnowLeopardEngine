#include "SnowLeopardEditor/EditorGUISystem.h"
#include "SnowLeopardEditor/EditorContext.h"
#include "SnowLeopardEditor/Panels/HierarchyPanel.h"
#include "SnowLeopardEditor/ImGuiRenderer.h"
#include "SnowLeopardEditor/PanelManager.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include <IconsMaterialDesignIcons.h>
#include <ImGuizmo.h>
#include <imgui.h>
#include <imgui_internal.h> // DockBuilder
#include <imgui_impl_sdl3.h>

#include <SDL3/SDL.h>

namespace SnowLeopardEngine::Editor
{
    EditorGUISystem::EditorGUISystem()  = default;
    EditorGUISystem::~EditorGUISystem() = default;

    void EditorGUISystem::Init(const EditorGUISystemInitInfo& initInfo)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
        // Multi-viewport (detached panels as OS windows) needs per-window VRI swapchains; not wired up yet.
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

        auto iniPath = FileSystem::GetExecutableRelativeDirectory("imgui.ini");
        if (FileSystem::Exists(iniPath))
        {
            ImGui::LoadIniSettingsFromDisk(iniPath.generic_string().c_str());
        }

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular
        // ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding              = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            style.PopupRounding = style.TabRounding = 6.0f;
        }

        // Add TTF Fonts & Icon Fonts
        ImFontConfig defaultConfig = {};
        io.Fonts->AddFontDefault(&defaultConfig);

        // https://github.com/ocornut/imgui/issues/3247
        static const ImWchar iconsRanges[] = {ICON_MIN_MDI, ICON_MAX_MDI, 0};
        ImFontConfig         iconsConfig   = defaultConfig;
        iconsConfig.MergeMode              = true;
        iconsConfig.PixelSnapH             = true;
        io.Fonts->AddFontFromFileTTF("Assets/Fonts/" FONT_ICON_FILE_NAME_MDI, 13, &iconsConfig, iconsRanges);

        SetupUIStyle();

        auto* window = g_EngineContext->WindowSys->GetWindow();
        // SDL3 for input and windowing; VRI's ImGui renderer draws (no GL / Vulkan specific ImGui backend).
        ImGui_ImplSDL3_InitForOther(static_cast<SDL_Window*>(window->GetPlatformWindow()));
        m_EventListener = window->AddNativeEventListener(
            [](const void* event) { ImGui_ImplSDL3_ProcessEvent(static_cast<const SDL_Event*>(event)); });
        auto& renderSystem = g_EngineContext->RenderSys;
        m_Renderer         = CreateScope<ImGuiRenderer>(renderSystem->GetDevice(), renderSystem->GetSwapchainFormat());
        renderSystem->AddOverlay(m_Renderer.get());

        // the edited scene, play mode, selection and camera (before the panels, which use them)
        EditorContext::Init(initInfo.ProjectFilePath);

        // init panel manager
        PanelManagerInitInfo panelInitInfo = {};
        panelInitInfo.ProjectFilePath      = initInfo.ProjectFilePath;
        PanelManager::Init(panelInitInfo);

        SNOW_LEOPARD_INFO("[EditorGUISystem] Initialized");
    }

    void EditorGUISystem::Shutdown()
    {
        SNOW_LEOPARD_INFO("[EditorGUISystem] Shutting Down...");
        PanelManager::Shutdown();
        EditorContext::Shutdown();
        g_EngineContext->WindowSys->GetWindow()->RemoveNativeEventListener(m_EventListener);
        g_EngineContext->RenderSys->RemoveOverlay(m_Renderer.get());
        m_Renderer.reset();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorGUISystem::OnFixedTick() { PanelManager::OnFixedTick(); }

    void EditorGUISystem::OnTick(float deltaTime)
    {
        EditorContext::OnTick();

        static bool               dockSpaceOpen  = true;
        static ImGuiDockNodeFlags dockSpaceFlags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        if (dockSpaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
        {
            windowFlags |= ImGuiWindowFlags_NoBackground;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpaceWindow", &dockSpaceOpen, windowFlags);
        ImGui::PopStyleVar(3);

        DrawMenuBar();
        HandleShortcuts();

        // DockSpace
        ImGuiIO&    io          = ImGui::GetIO();
        ImGuiStyle& style       = ImGui::GetStyle();
        float       minWinSizeX = style.WindowMinSize.x;
        float       minWinSizeY = style.WindowMinSize.y;
        style.WindowMinSize.x   = 350.0f;
        style.WindowMinSize.y   = 120.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            // "DockSpace2": the layout gained the Hierarchy and Render Settings panels; a layout saved for the old
            // dock space would leave them floating.
            ImGuiID dockSpaceId = ImGui::GetID("DockSpace2");
            BuildDefaultLayout(dockSpaceId, viewport->Size);
            ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), dockSpaceFlags);
        }
        style.WindowMinSize.x = minWinSizeX;
        style.WindowMinSize.y = minWinSizeY;

        if (auto& panels = EditorContext::GetPanels(); panels.ImGuiDemo)
        {
            ImGui::ShowDemoWindow(&panels.ImGuiDemo);
        }

        PanelManager::OnTick(deltaTime);

        ImGui::End();
    }

    void EditorGUISystem::BuildDefaultLayout(uint32_t dockSpaceId, const ImVec2& size)
    {
        // Built in code rather than shipped as an imgui.ini: dock node IDs saved by one imgui version did not match
        // the IDs 1.92 computes, which left every panel floating. A layout the user saved (a split dock space) wins.
        static bool checked = false;
        if (checked)
        {
            return;
        }
        checked = true;

        ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockSpaceId);
        if (node != nullptr && node->IsSplitNode())
        {
            return;
        }

        ImGui::DockBuilderRemoveNode(dockSpaceId);
        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockSpaceId, size);

        // Hierarchy | Viewport | Inspector over Render Settings, Console + Resources below the Viewport.
        ImGuiID center      = dockSpaceId;
        ImGuiID right       = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.24f, nullptr, &center);
        ImGuiID rightBottom = ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, 0.42f, nullptr, &right);
        ImGuiID left        = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.2f, nullptr, &center);
        ImGuiID bottom      = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.28f, nullptr, &center);

        ImGui::DockBuilderDockWindow("Viewport", center);
        ImGui::DockBuilderDockWindow("###Hierarchy", left);
        ImGui::DockBuilderDockWindow("Inspector", right);
        ImGui::DockBuilderDockWindow("###RenderSettings", rightBottom);
        ImGui::DockBuilderDockWindow("Dear ImGui Demo", right);
        ImGui::DockBuilderDockWindow("###Resources", bottom);
        ImGui::DockBuilderDockWindow("Console", bottom);
        ImGui::DockBuilderGetNode(bottom)->SelectedTabId = ImHashStr("Console"); // front tab (window id = name hash)
        ImGui::DockBuilderFinish(dockSpaceId);
    }

    void EditorGUISystem::DrawMenuBar()
    {
        if (!ImGui::BeginMenuBar())
        {
            return;
        }
        const auto mode    = EditorContext::GetMode();
        const bool editing = mode == EditorMode::Edit;

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem(ICON_MDI_FILE_PLUS " New Scene", "Ctrl+N"))
                EditorContext::NewScene();
            if (ImGui::MenuItem(ICON_MDI_FOLDER_OPEN " Open Scene...", "Ctrl+O"))
                EditorContext::ShowOpenSceneDialog();
            ImGui::Separator();
            // Saving always writes the edited scene (not the simulated copy).
            if (ImGui::MenuItem(ICON_MDI_CONTENT_SAVE " Save Scene", "Ctrl+S"))
                EditorContext::SaveScene();
            if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
                EditorContext::ShowSaveSceneDialog();
            ImGui::Separator();
            if (ImGui::MenuItem(ICON_MDI_EXIT_TO_APP " Exit"))
                m_WantsQuit = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            const bool any = !EditorContext::GetSelection().empty();
            if (ImGui::MenuItem(ICON_MDI_CONTENT_COPY " Duplicate", "Ctrl+D", false, any))
                EditorContext::DuplicateSelection();
            if (ImGui::MenuItem(ICON_MDI_DELETE " Delete", "Del", false, any))
                EditorContext::DeleteSelection();
            if (ImGui::MenuItem("Deselect All", "Esc", false, any))
                EditorContext::ClearSelection();
            ImGui::Separator();
            if (ImGui::MenuItem("Frame Selection", "F", false, any))
                EditorContext::FocusSelection();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Entity"))
        {
            HierarchyPanel::DrawCreateMenu({});
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Play"))
        {
            if (ImGui::MenuItem(mode == EditorMode::Paused ? "Resume" : "Play", "Ctrl+P", false, mode != EditorMode::Playing))
                EditorContext::Play();
            if (ImGui::MenuItem("Pause", nullptr, false, mode == EditorMode::Playing))
                EditorContext::Pause();
            if (ImGui::MenuItem("Stop", "Ctrl+P", false, !editing))
                EditorContext::Stop();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            auto& panels = EditorContext::GetPanels();
            ImGui::MenuItem("Hierarchy", nullptr, &panels.Hierarchy);
            ImGui::MenuItem("Inspector", nullptr, &panels.Inspector);
            ImGui::MenuItem("Render Settings", nullptr, &panels.RenderSettings);
            ImGui::MenuItem("Resources", nullptr, &panels.Resources);
            ImGui::MenuItem("Console", nullptr, &panels.Console);
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo", nullptr, &panels.ImGuiDemo);
            ImGui::EndMenu();
        }

        // Right: the scene file and the mode.
        std::string status = EditorContext::GetSceneTitle();
        if (!editing)
            status += mode == EditorMode::Playing ? "   " ICON_MDI_PLAY " Playing" : "   " ICON_MDI_PAUSE " Paused";
        const float width = ImGui::CalcTextSize(status.c_str()).x;
        ImGui::SameLine(std::max(ImGui::GetCursorPosX() + 20, ImGui::GetWindowWidth() - width - 16));
        if (editing)
            ImGui::TextDisabled("%s", status.c_str());
        else
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "%s", status.c_str());
        if (ImGui::IsItemHovered() && !EditorContext::GetScenePath().empty())
            ImGui::SetTooltip("%s", EditorContext::GetScenePath().generic_string().c_str());

        ImGui::EndMenuBar();
    }

    void EditorGUISystem::HandleShortcuts()
    {
        // Global (any panel focused), except while typing into a text field.
        if (ImGui::GetIO().WantTextInput)
        {
            return;
        }
        constexpr ImGuiInputFlags global = ImGuiInputFlags_RouteGlobal;
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_N, global))
            EditorContext::NewScene();
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, global))
            EditorContext::ShowOpenSceneDialog();
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S, global))
            EditorContext::ShowSaveSceneDialog();
        else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, global))
            EditorContext::SaveScene();
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_D, global))
            EditorContext::DuplicateSelection();
        if (ImGui::Shortcut(ImGuiKey_Delete, global))
            EditorContext::DeleteSelection();
        if (ImGui::Shortcut(ImGuiKey_Escape, global))
            EditorContext::ClearSelection();
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_P, global))
        {
            if (EditorContext::GetMode() == EditorMode::Edit)
                EditorContext::Play();
            else
                EditorContext::Stop();
        }
    }

    void EditorGUISystem::Begin()
    {
        // High DPI scaling
        ImGui::GetIO().FontGlobalScale = g_EngineContext->WindowSys->GetContentScale();

        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }

    void EditorGUISystem::End()
    {
        ImGuiIO& io    = ImGui::GetIO();
        io.DisplaySize = ImVec2(g_EngineContext->WindowSys->GetWidth(), g_EngineContext->WindowSys->GetHeight());

        // Build the draw data; the render system uploads and draws it (ImGuiRenderer) when it presents.
        ImGui::Render();
    }

    void EditorGUISystem::SetupUIStyle()
    {
        // Clean Dark/Red style by ImBritish from ImThemes
        ImGuiStyle& style = ImGui::GetStyle();

        style.Alpha                     = 1.0f;
        style.DisabledAlpha             = 0.6000000238418579f;
        style.WindowPadding             = ImVec2(8.0f, 8.0f);
        style.WindowRounding            = 0.0f;
        style.WindowBorderSize          = 1.0f;
        style.WindowMinSize             = ImVec2(32.0f, 32.0f);
        style.WindowTitleAlign          = ImVec2(0.0f, 0.5f);
        style.WindowMenuButtonPosition  = ImGuiDir_Left;
        style.ChildRounding             = 0.0f;
        style.ChildBorderSize           = 1.0f;
        style.PopupRounding             = 0.0f;
        style.PopupBorderSize           = 1.0f;
        style.FramePadding              = ImVec2(4.0f, 3.0f);
        style.FrameRounding             = 0.0f;
        style.FrameBorderSize           = 1.0f;
        style.ItemSpacing               = ImVec2(8.0f, 4.0f);
        style.ItemInnerSpacing          = ImVec2(4.0f, 4.0f);
        style.CellPadding               = ImVec2(4.0f, 2.0f);
        style.IndentSpacing             = 21.0f;
        style.ColumnsMinSpacing         = 6.0f;
        style.ScrollbarSize             = 14.0f;
        style.ScrollbarRounding         = 9.0f;
        style.GrabMinSize               = 10.0f;
        style.GrabRounding              = 0.0f;
        style.TabRounding               = 0.0f;
        style.TabBorderSize             = 1.0f;
        style.TabCloseButtonMinWidthUnselected = 0.0f; // was TabMinWidthForCloseButton (renamed in imgui 1.91.9)
        style.ColorButtonPosition       = ImGuiDir_Right;
        style.ButtonTextAlign           = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign       = ImVec2(0.0f, 0.0f);

        style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_TextDisabled] =
            ImVec4(0.729411780834198f, 0.7490196228027344f, 0.7372549176216125f, 1.0f);
        style.Colors[ImGuiCol_WindowBg] =
            ImVec4(0.08627451211214066f, 0.08627451211214066f, 0.08627451211214066f, 0.9399999976158142f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_PopupBg] =
            ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f, 0.9399999976158142f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
        style.Colors[ImGuiCol_BorderShadow] =
            ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5400000214576721f);
        style.Colors[ImGuiCol_FrameBgHovered] =
            ImVec4(0.1764705926179886f, 0.1764705926179886f, 0.1764705926179886f, 0.4000000059604645f);
        style.Colors[ImGuiCol_FrameBgActive] =
            ImVec4(0.2156862765550613f, 0.2156862765550613f, 0.2156862765550613f, 0.6700000166893005f);
        style.Colors[ImGuiCol_TitleBg] =
            ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 0.6523605585098267f);
        style.Colors[ImGuiCol_TitleBgActive] =
            ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
        style.Colors[ImGuiCol_TitleBgCollapsed] =
            ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 0.6700000166893005f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2156862765550613f, 0.2156862765550613f, 0.2156862765550613f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarBg] =
            ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
        style.Colors[ImGuiCol_ScrollbarGrab] =
            ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] =
            ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] =
            ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
        style.Colors[ImGuiCol_CheckMark]        = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_SliderGrab]       = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.3803921639919281f, 0.3803921639919281f, 1.0f);
        style.Colors[ImGuiCol_Button]           = ImVec4(0.0f, 0.0f, 0.0f, 0.5411764979362488f);
        style.Colors[ImGuiCol_ButtonHovered] =
            ImVec4(0.1764705926179886f, 0.1764705926179886f, 0.1764705926179886f, 0.4000000059604645f);
        style.Colors[ImGuiCol_ButtonActive] =
            ImVec4(0.2156862765550613f, 0.2156862765550613f, 0.2156862765550613f, 0.6705882549285889f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.2156862765550613f, 0.2156862765550613f, 0.2156862765550613f, 1.0f);
        style.Colors[ImGuiCol_HeaderHovered] =
            ImVec4(0.2705882489681244f, 0.2705882489681244f, 0.2705882489681244f, 1.0f);
        style.Colors[ImGuiCol_HeaderActive] =
            ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3529411852359772f, 1.0f);
        style.Colors[ImGuiCol_Separator]         = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_SeparatorHovered]  = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_SeparatorActive]   = ImVec4(1.0f, 0.3294117748737335f, 0.3294117748737335f, 1.0f);
        style.Colors[ImGuiCol_ResizeGrip]        = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 0.4862745106220245f, 0.4862745106220245f, 1.0f);
        style.Colors[ImGuiCol_ResizeGripActive]  = ImVec4(1.0f, 0.4862745106220245f, 0.4862745106220245f, 1.0f);
        style.Colors[ImGuiCol_Tab]        = ImVec4(0.2196078449487686f, 0.2196078449487686f, 0.2196078449487686f, 1.0f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2901960909366608f, 0.2901960909366608f, 0.2901960909366608f, 1.0f);
        style.Colors[ImGuiCol_TabSelected] = ImVec4(0.1764705926179886f, 0.1764705926179886f, 0.1764705926179886f, 1.0f);
        style.Colors[ImGuiCol_TabDimmed] =
            ImVec4(0.1490196138620377f, 0.06666667014360428f, 0.06666667014360428f, 0.9700000286102295f);
        style.Colors[ImGuiCol_TabDimmedSelected] =
            ImVec4(0.4039215743541718f, 0.1529411822557449f, 0.1529411822557449f, 1.0f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
        style.Colors[ImGuiCol_PlotLinesHovered]     = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_PlotHistogram]        = ImVec4(0.8980392217636108f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.364705890417099f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_TableHeaderBg] =
            ImVec4(0.3019607961177826f, 0.3019607961177826f, 0.3019607961177826f, 1.0f);
        style.Colors[ImGuiCol_TableBorderStrong] =
            ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
        style.Colors[ImGuiCol_TableBorderLight] =
            ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
        style.Colors[ImGuiCol_TableRowBg]    = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
        style.Colors[ImGuiCol_TextSelectedBg] =
            ImVec4(0.2627451121807098f, 0.6352941393852234f, 0.8784313797950745f, 0.4377682209014893f);
        style.Colors[ImGuiCol_DragDropTarget] =
            ImVec4(0.4666666686534882f, 0.1843137294054031f, 0.1843137294054031f, 0.9656652212142944f);
        style.Colors[ImGuiCol_NavCursor] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
        style.Colors[ImGuiCol_NavWindowingDimBg] =
            ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
        style.Colors[ImGuiCol_ModalWindowDimBg] =
            ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
    }
} // namespace SnowLeopardEngine::Editor