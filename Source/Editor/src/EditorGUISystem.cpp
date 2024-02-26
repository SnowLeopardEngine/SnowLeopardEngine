#include "SnowLeopardEditor/EditorGUISystem.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"

#include <GLFW/glfw3.h>
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

        // TODO: TTF Fonts & Icon Fonts

        SetupUIStyle();

        ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(g_EngineContext->WindowSys->GetPlatformWindow()), true);
        ImGui_ImplOpenGL3_Init("#version 330");

        SNOW_LEOPARD_INFO("[EditorGUISystem] Initialized");
    }

    void EditorGUISystem::Shutdown()
    {
        SNOW_LEOPARD_INFO("[EditorGUISystem] Shutting Down...");
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorGUISystem::OnTick(float deltaTime)
    {
        g_EngineContext->RenderSys->GetAPI()->ClearColor(glm::vec4(0, 0, 0, 0), ClearBit::Default);
        ImGui::ShowDemoWindow();
    }

    void EditorGUISystem::Begin()
    {
        // High DPI scaling
        float dpiScaleX, dpiScaleY;
        glfwGetWindowContentScale(static_cast<GLFWwindow*>(g_EngineContext->WindowSys->GetPlatformWindow()), &dpiScaleX, &dpiScaleY);
        ImGui::GetIO().FontGlobalScale = dpiScaleX;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
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