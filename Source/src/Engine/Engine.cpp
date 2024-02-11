#include "SnowLeopardEngine/Engine/Engine.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"

namespace SnowLeopardEngine
{
    bool Engine::Init()
    {
        // Init engine context
        g_EngineContext = new EngineContext();
        g_EngineContext->LogSys.Init();

        WindowSystemInitInfo windowSysInitInfo {};
        g_EngineContext->WindowSys.Init(windowSysInitInfo);

        SNOW_LEOPARD_CORE_INFO("[Engine] Initialized");

        return true;
    }

    void Engine::TickOneFrame(float deltaTime)
    {
        // Tick sub systems
        SNOW_LEOPARD_CORE_INFO("[Engine] DeltaTime: {0}", deltaTime);

        if (!g_EngineContext->WindowSys->Tick())
        {
            return;
        }
    }

    void Engine::Shutdown()
    {
        SNOW_LEOPARD_CORE_INFO("[Engine] Shutting Down...");

        g_EngineContext->WindowSys.Shutdown();
        g_EngineContext->LogSys.Shutdown();
    }

    EngineContext* Engine::GetContext() { return g_EngineContext; }
} // namespace SnowLeopardEngine
