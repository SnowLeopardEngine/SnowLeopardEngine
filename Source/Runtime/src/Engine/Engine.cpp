#include "SnowLeopardEngine/Engine/Engine.h"
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

        // Init camera system
        g_EngineContext->CameraSys.Init();

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

        if (!g_EngineContext->WindowSys->IsMinimized())
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

        for (auto& lifeTime : m_LiftTimeComponents)
        {
            lifeTime->OnUnload();
        }

        g_EngineContext->CameraSys.Shutdown();
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
} // namespace SnowLeopardEngine
