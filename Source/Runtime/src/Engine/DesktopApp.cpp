#include "SnowLeopardEngine/Engine/DesktopApp.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Event/WindowEvents.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Core/Event/SceneEvents.h"

namespace SnowLeopardEngine
{
    DesktopApp* DesktopApp::s_Instance = nullptr;

    DesktopApp::DesktopApp(int argc, char** argv)
    {
        FileSystem::InitExecutableDirectory(argv[0]);
        s_Instance = this;
    }

    bool DesktopApp::Init(const DesktopAppInitInfo& initInfo)
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
        Subscribe(m_WindowResizeHandler);      

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
            Time::ElapsedTime += dt;

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
        Unsubscribe(m_WindowResizeHandler);
        Unsubscribe(m_WindowCloseHandler);

        // shutdown the engine
        m_Engine->Shutdown();
    }

    void DesktopApp::OnWindowClose(const WindowCloseEvent& e)
    {
        SNOW_LEOPARD_CORE_INFO("[App] OnWindowClose");
        Quit();
    }

    void DesktopApp::OnWindowResize(const WindowResizeEvent& e)
    {
        SNOW_LEOPARD_CORE_INFO("[App] OnWindowResize, {0}", e.ToString());

        auto w = e.GetWidth();
        auto h = e.GetHeight();

        SNOW_LEOPARD_CORE_ASSERT(w != 0 && h != 0, "[App] Window size must be greater than (0, 0)");
        g_EngineContext->RenderSys->UpdateViewport({
            .Offset = {.X = 0, .Y = 0},
            .Extent = {.Width = static_cast<uint32_t>(w), .Height = static_cast<uint32_t>(h)},
        });
    }
} // namespace SnowLeopardEngine
