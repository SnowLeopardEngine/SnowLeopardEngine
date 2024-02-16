#include "SnowLeopardEngine/Engine/Engine.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Time/Time.h"

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

        // Init window system
        WindowSystemInitInfo windowSysInitInfo {};
        windowSysInitInfo.Window = initInfo.Window;
        g_EngineContext->WindowSys.Init(windowSysInitInfo);

        // Init scene manager
        g_EngineContext->SceneMngr.Init();

        // Init audio system
        g_EngineContext->AudioSys.Init();

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

        // Tick Logic
        g_EngineContext->SceneMngr->OnTick(deltaTime);

        // TODO: Tick Rendering

        for (auto& lifeTime : m_LiftTimeComponents)
        {
            lifeTime->OnTick(deltaTime);
        }
    }

    void Engine::FixedTickOneFrame()
    {
        // SNOW_LEOPARD_CORE_INFO("[Engine] FixedTick, FixedDeltaTime: {0}", Time::FixedDeltaTime);

        // Tick Physics
        g_EngineContext->SceneMngr->OnFixedTick();

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

        g_EngineContext->AudioSys.Shutdown();
        g_EngineContext->SceneMngr->OnUnload();
        g_EngineContext->SceneMngr.Shutdown();
        g_EngineContext->WindowSys.Shutdown();
        g_EngineContext->EventSys.Shutdown();
        g_EngineContext->LogSys.Shutdown();
    }

    EngineContext* Engine::GetContext() { return g_EngineContext; }
} // namespace SnowLeopardEngine
