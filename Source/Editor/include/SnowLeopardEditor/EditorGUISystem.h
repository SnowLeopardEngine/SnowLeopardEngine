#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

struct ImVec2;

namespace SnowLeopardEngine::Editor
{
    class ImGuiRenderer;

    struct EditorGUISystemInitInfo
    {
        std::string ProjectFilePath;
    };

    class EditorGUISystem
    {
    public:
        EditorGUISystem();
        ~EditorGUISystem(); // defined where ImGuiRenderer is complete

        void Init(const EditorGUISystemInitInfo& initInfo);
        void Shutdown();

        void OnFixedTick();
        void OnTick(float deltaTime);

        void Begin();
        void End();

    private:
        void SetupUIStyle();
        void BuildDefaultLayout(uint32_t dockSpaceId, const ImVec2& size);

    private:
        uint32_t             m_EventListener = 0; // Window::NativeEventListenerId feeding SDL events to ImGui
        Scope<ImGuiRenderer> m_Renderer;          // draws ImGui in the render system's swapchain pass
    };
} // namespace SnowLeopardEngine::Editor