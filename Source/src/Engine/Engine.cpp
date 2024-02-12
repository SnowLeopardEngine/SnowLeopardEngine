#include "SnowLeopardEngine/Engine/Engine.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"

namespace SnowLeopardEngine
{
    bool Engine::Init()
    {
        // Init engine context
        g_EngineContext = new EngineContext();

        // Init log system
        g_EngineContext->LogSys.Init();

        // Init event system
        g_EngineContext->EventSys.Init();

        // Init window system
        WindowSystemInitInfo windowSysInitInfo {};
        g_EngineContext->WindowSys.Init(windowSysInitInfo);

        SNOW_LEOPARD_CORE_INFO("[Engine] Initialized");

        return true;
    }

    void Engine::TickOneFrame(float deltaTime)
    {
        // Tick sub systems
        SNOW_LEOPARD_CORE_INFO("[Engine] DeltaTime: {0}", deltaTime);

        // Tick Window System
        if (!g_EngineContext->WindowSys->Tick())
        {
            return;
        }

        // Dispatch Events
        g_EngineContext->EventSys->DispatchEvents();

        // TODO: Tick Logic

        // TODO: Tick Rendering
    }

    void Engine::Shutdown()
    {
        SNOW_LEOPARD_CORE_INFO("[Engine] Shutting Down...");

        g_EngineContext->WindowSys.Shutdown();
        g_EngineContext->EventSys.Shutdown();
        g_EngineContext->LogSys.Shutdown();
    }

    EngineContext* Engine::GetContext() { return g_EngineContext; }
} // namespace SnowLeopardEngine
