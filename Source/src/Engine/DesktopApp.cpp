#include "SnowLeopardEngine/Engine/DesktopApp.h"
#include "SnowLeopardEngine/Core/Event/EventUtil.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
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

        // subscribe events
        Subscribe(m_WindowCloseHandler);

        m_IsRunning = true;

        return true;
    }

    bool DesktopApp::PostInit()
    {
        if (!m_Engine->PostInit())
        {
            std::cerr << "Failed to post initialize the engine!" << std::endl;
            return false;
        }

        return true;
    }

    void DesktopApp::Run()
    {
        float fixedTimer = 0;
        while (m_IsRunning && !m_Engine->GetContext()->WindowSys->ShouldClose())
        {
            m_Timer.Start();
            float dt        = m_Timer.GetDeltaTime();
            Time::DeltaTime = dt;

            // Fixed Tick
            fixedTimer += dt;
            while (fixedTimer >= Time::FixedDeltaTime)
            {
                m_Engine->FixedTickOneFrame();

                fixedTimer -= Time::FixedDeltaTime;
            }

            // Normal Tick
            m_Engine->TickOneFrame(dt);

            m_Timer.Stop();
        }

        Shutdown();
    }

    void DesktopApp::Quit() { m_IsRunning = false; }

    void DesktopApp::Shutdown()
    {
        // unsubscribe events
        Unsubscribe(m_WindowCloseHandler);

        // shutdown the engine
        m_Engine->Shutdown();
    }

    void DesktopApp::OnWindowClose(const WindowCloseEvent& e)
    {
        SNOW_LEOPARD_CORE_INFO("[App] OnWindowClose");
        Quit();
    }
} // namespace SnowLeopardEngine
