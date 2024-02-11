#include "SnowLeopardEngine/Engine/DesktopApp.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    DesktopApp* DesktopApp::s_Instance = nullptr;

    DesktopApp::DesktopApp() { s_Instance = this; }

    bool DesktopApp::Init()
    {
        // create the engine
        m_Engine = CreateRef<Engine>();
        if (!m_Engine->Init())
        {
            std::cerr << "Failed to initialize the engine!" << std::endl;
            return false;
        }

        m_IsRunning = true;

        return true;
    }

    void DesktopApp::Run()
    {
        while (m_IsRunning)
        {
            m_Timer.Start();
            m_Engine->TickOneFrame(m_Timer.GetDeltaTime());
            m_Timer.Stop();
        }

        Shutdown();
    }

    void DesktopApp::Quit() { m_IsRunning = false; }

    void DesktopApp::Shutdown() { m_Engine->Shutdown(); }
} // namespace SnowLeopardEngine
