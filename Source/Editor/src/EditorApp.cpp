#include "SnowLeopardEditor/EditorApp.h"
#include "SnowLeopardEngine/Core/Event/EventUtil.h"
#include "SnowLeopardEngine/Engine/DesktopApp.h"

namespace SnowLeopardEngine::Editor
{
    EditorApp* EditorApp::s_Instance = nullptr;

    EditorApp::EditorApp(int argc, char** argv) : DesktopApp(argc, argv) { s_Instance = this; }

    bool EditorApp::Init(const EditorAppInitInfo& initInfo)
    {
        // create the engine
        m_Engine = CreateRef<Engine>();
        if (!m_Engine->Init(initInfo.Engine))
        {
            std::cerr << "Failed to initialize the engine!" << std::endl;
            return false;
        }

        // subscribe events
        Subscribe(m_WindowCloseHandler);

        m_IsRunning = true;

        SNOW_LEOPARD_INFO("[Editor] Initialized");

        return true;
    }

    bool EditorApp::PostInit()
    {
        if (!m_Engine->PostInit())
        {
            std::cerr << "Failed to post initialize the engine!" << std::endl;
            return false;
        }

        return true;
    }

    void EditorApp::Run()
    {
        while (m_IsRunning && !m_Engine->GetContext()->WindowSys->ShouldClose())
        {
            // Tick Window System
            if (!m_Engine->GetContext()->WindowSys->Tick())
            {
                return;
            }

            // Dispatch Events
            m_Engine->GetContext()->EventSys->DispatchEvents();

            // GUI Begin

            // GUI Draw

            // GUI End

            m_Engine->GetContext()->RenderSys->Present();
        }

        Shutdown();
    }

    void EditorApp::Shutdown()
    {
        SNOW_LEOPARD_INFO("[Editor] Shutting Down...");

        // GUI Shutdown

        DesktopApp::Shutdown();
    }
} // namespace SnowLeopardEngine::Editor