#include "SnowLeopardEngine/Engine/Engine.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Event/EventUtil.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    bool Engine::Init(const EngineInitInfo& initInfo)
    {
        // Init engine context
        g_EngineContext = new EngineContext();

        // Init log system
        g_EngineContext->LogSys.Init();

        // Init event system
        g_EngineContext->EventSys.Init();

        // Init audio system
        g_EngineContext->AudioSys.Init();

        // Init physics system
        g_EngineContext->PhysicsSys.Init();

        // Init window system
        WindowSystemInitInfo windowSysInitInfo {};
        windowSysInitInfo.Window = initInfo.Window;
        g_EngineContext->WindowSys.Init(windowSysInitInfo);

        // Init input system
        g_EngineContext->InputSys.Init();

        // Init scene manager
        g_EngineContext->SceneMngr.Init();

        // Init render system
        g_EngineContext->RenderSys.Init();

        // subscribe events
        Subscribe(m_WindowResizeHandler);

        SNOW_LEOPARD_CORE_INFO("[Engine] Initialized");

        return true;
    }

    bool Engine::PostInit()
    {
        // LifeTimeComponents OnLoad
        for (auto& lifeTime : m_LiftTimeComponents)
        {
            lifeTime->OnLoad();
        }

        g_EngineContext->SceneMngr->OnLoad();

        return true;
    }

    void Engine::TickOneFrame(float deltaTime)
    {
        // Tick sub systems
        // SNOW_LEOPARD_CORE_INFO("[Engine] DeltaTime: {0}", deltaTime);

        // Tick Window System
        if (!g_EngineContext->WindowSys->Tick())
        {
            return;
        }

        // Dispatch Events
        g_EngineContext->EventSys->DispatchEvents();

        for (auto& lifeTime : m_LiftTimeComponents)
        {
            lifeTime->OnTick(deltaTime);
        }

        // Tick Logic
        g_EngineContext->SceneMngr->OnTick(deltaTime);

        if (!m_IsWindowMinimized)
        {
            // Tick Rendering
            g_EngineContext->RenderSys->OnTick(deltaTime);

            // Present
            g_EngineContext->RenderSys->Present();
        }
    }

    void Engine::FixedTickOneFrame()
    {
        // SNOW_LEOPARD_CORE_INFO("[Engine] FixedTick, FixedDeltaTime: {0}", Time::FixedDeltaTime);

        // Tick Logic
        g_EngineContext->SceneMngr->OnFixedTick();

        // Tick Physics
        g_EngineContext->PhysicsSys->OnFixedTick();

        for (auto& lifeTime : m_LiftTimeComponents)
        {
            lifeTime->OnFixedTick();
        }
    }

    void Engine::Shutdown()
    {
        SNOW_LEOPARD_CORE_INFO("[Engine] Shutting Down...");

        // unsubscribe events
        Unsubscribe(m_WindowResizeHandler);

        for (auto& lifeTime : m_LiftTimeComponents)
        {
            lifeTime->OnUnload();
        }

        g_EngineContext->RenderSys.Shutdown();
        g_EngineContext->SceneMngr->OnUnload();
        g_EngineContext->SceneMngr.Shutdown();
        g_EngineContext->InputSys.Shutdown();
        g_EngineContext->WindowSys.Shutdown();
        g_EngineContext->PhysicsSys.Shutdown();
        g_EngineContext->AudioSys.Shutdown();
        g_EngineContext->EventSys.Shutdown();
        g_EngineContext->LogSys.Shutdown();
    }

    EngineContext* Engine::GetContext() { return g_EngineContext; }

    void Engine::OnWindowResize(const WindowResizeEvent& e)
    {
        SNOW_LEOPARD_CORE_INFO("[App] OnWindowResize, {0}", e.ToString());

        auto w = e.GetWidth();
        auto h = e.GetHeight();

        if (w == 0 || h == 0)
        {
            m_IsWindowMinimized = true;
            return;
        }

        m_IsWindowMinimized = false;
        g_EngineContext->RenderSys->GetAPI()->UpdateViewport(0, 0, w, h);
    }
} // namespace SnowLeopardEngine
