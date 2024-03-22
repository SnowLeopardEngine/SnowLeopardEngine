#include "SnowLeopardEditor/EditorApp.h"
#include "SnowLeopardEditor/EditorGUISystem.h"
#include "SnowLeopardEditor/PanelManager.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
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

        // init GUI system
        m_GUISystem.Init();

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
        float fixedTimer = 0;
        while (m_IsRunning && !m_Engine->GetContext()->WindowSys->ShouldClose())
        {
            m_Timer.Start();
            float dt        = m_Timer.GetDeltaTime();
            Time::DeltaTime = dt;
            Time::ElapsedTime += dt;

            // Fixed Tick
            fixedTimer += dt;
            while (fixedTimer >= Time::FixedDeltaTime)
            {
                m_GUISystem.OnFixedTick();
                fixedTimer -= Time::FixedDeltaTime;
            }

            // Tick Window System
            if (!m_Engine->GetContext()->WindowSys->Tick())
            {
                return;
            }

            // Dispatch Events
            m_Engine->GetContext()->EventSys->DispatchEvents();

            // GUI Begin
            m_GUISystem.Begin();

            // GUI Draw
            m_GUISystem.OnTick(dt);

            // GUI End
            m_GUISystem.End();

            m_Engine->GetContext()->RenderSys->Present();

            m_Timer.Stop();
        }

        Shutdown();
    }

    void EditorApp::Shutdown()
    {
        SNOW_LEOPARD_INFO("[Editor] Shutting Down...");

        // GUI Shutdown
        m_GUISystem.Shutdown();

        DesktopApp::Shutdown();
    }
} // namespace SnowLeopardEngine::Editor