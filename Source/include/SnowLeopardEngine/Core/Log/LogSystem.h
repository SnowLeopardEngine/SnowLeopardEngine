#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"

// clang-format off
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
// clang-format on

namespace SnowLeopardEngine
{
    class LogSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(LogSystem)

        Ref<spdlog::logger>& GetCoreLogger() { return m_CoreLogger; }
        Ref<spdlog::logger>& GetClientLogger() { return m_ClientLogger; }

    private:
        Ref<spdlog::logger> m_CoreLogger;
        Ref<spdlog::logger> m_ClientLogger;
    };
} // namespace SnowLeopardEngine

// Core log macros
#define SNOW_LEOPARD_CORE_TRACE(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetCoreLogger()->trace(__VA_ARGS__)
#define SNOW_LEOPARD_CORE_INFO(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetCoreLogger()->info(__VA_ARGS__)
#define SNOW_LEOPARD_CORE_WARN(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetCoreLogger()->warn(__VA_ARGS__)
#define SNOW_LEOPARD_CORE_ERROR(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetCoreLogger()->error(__VA_ARGS__)
#define SNOW_LEOPARD_CORE_CRITICAL(...) \
    ::SnowLeopardEngine::g_EngineContext->LogSys->GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define SNOW_LEOPARD_TRACE(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetClientLogger()->trace(__VA_ARGS__)
#define SNOW_LEOPARD_INFO(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetClientLogger()->info(__VA_ARGS__)
#define SNOW_LEOPARD_WARN(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetClientLogger()->warn(__VA_ARGS__)
#define SNOW_LEOPARD_ERROR(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetClientLogger()->error(__VA_ARGS__)
#define SNOW_LEOPARD_CRITICAL(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetClientLogger()->critical(__VA_ARGS__)