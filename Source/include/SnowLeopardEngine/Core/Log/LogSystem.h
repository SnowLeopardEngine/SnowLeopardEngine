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