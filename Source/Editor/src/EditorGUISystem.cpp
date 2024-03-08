#include "SnowLeopardEditor/EditorGUISystem.h"
#include "SnowLeopardEditor/PanelManager.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"

#include <GLFW/glfw3.h>
#include <IconsMaterialDesignIcons.h>
#include <ImGuizmo.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace SnowLeopardEngine::Editor
{
    void EditorGUISystem::Init()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
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
        io.Fonts->AddFontFromFileTTF("Assets/Fonts/" FONT_ICON_FILE_NAME_MDI, 12, &iconsConfig, iconsRanges);

        SetupUIStyle();

        ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(g_EngineContext->WindowSys->GetPlatformWindow()), true);
        ImGui_ImplOpenGL3_Init("#version 330");

        // init panel manager
        PanelManager::Init();

        SNOW_LEOPARD_INFO("[EditorGUISystem] Initialized");
    }

    void EditorGUISystem::Shutdown()
    {
        SNOW_LEOPARD_INFO("[EditorGUISystem] Shutting Down...");
        PanelManager::Shutdown();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorGUISystem::OnFixedTick() { PanelManager::OnFixedTick(); }

    void EditorGUISystem::OnTick(float deltaTime)
    {
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

        // DockSpace
        ImGuiIO&    io          = ImGui::GetIO();
        ImGuiStyle& style       = ImGui::GetStyle();
        float       minWinSizeX = style.WindowMinSize.x;
        float       minWinSizeY = style.WindowMinSize.y;
        style.WindowMinSize.x   = 350.0f;
        style.WindowMinSize.y   = 120.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockSpaceId = ImGui::GetID("DockSpace");
            ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), dockSpaceFlags);
        }
        style.WindowMinSize.x = minWinSizeX;
        style.WindowMinSize.y = minWinSizeY;

        g_EngineContext->RenderSys->GetAPI()->ClearColor(glm::vec4(0, 0, 0, 0), ClearBit::Default);
        ImGui::ShowDemoWindow();

        PanelManager::OnTick(deltaTime);

        ImGui::End();
    }

    void EditorGUISystem::Begin()
    {
        // High DPI scaling
        float dpiScaleX, dpiScaleY;
        glfwGetWindowContentScale(
            static_cast<GLFWwindow*>(g_EngineContext->WindowSys->GetPlatformWindow()), &dpiScaleX, &dpiScaleY);
        ImGui::GetIO().FontGlobalScale = dpiScaleX;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }

    void EditorGUISystem::End()
    {
        ImGuiIO& io    = ImGui::GetIO();
        io.DisplaySize = ImVec2(g_EngineContext->WindowSys->GetWidth(), g_EngineContext->WindowSys->GetHeight());

        // Rendering
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupCurrentContext);
        }
    }

    void EditorGUISystem::SetupUIStyle() {}
} // namespace SnowLeopardEngine::Editor