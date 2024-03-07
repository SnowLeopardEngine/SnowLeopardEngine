#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Log/LogTypeDef.h"

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

        template<typename... Args>
        void Trace(bool isCore, Args&&... args)
        {
            GetLogger(isCore)->trace(args...);
            TriggerLogEvent(GetRegion(isCore), LogLevel::Trace, fmt::format(args...));
        }

        template<typename... Args>
        void Info(bool isCore, Args&&... args)
        {
            GetLogger(isCore)->info(args...);
            TriggerLogEvent(GetRegion(isCore), LogLevel::Info, fmt::format(args...));
        }

        template<typename... Args>
        void Warn(bool isCore, Args&&... args)
        {
            GetLogger(isCore)->warn(args...);
            TriggerLogEvent(GetRegion(isCore), LogLevel::Warn, fmt::format(args...));
        }

        template<typename... Args>
        void Error(bool isCore, Args&&... args)
        {
            GetLogger(isCore)->error(args...);
            TriggerLogEvent(GetRegion(isCore), LogLevel::Error, fmt::format(args...));
        }

        template<typename... Args>
        void Critical(bool isCore, Args&&... args)
        {
            GetLogger(isCore)->critical(args...);
            TriggerLogEvent(GetRegion(isCore), LogLevel::Critical, fmt::format(args...));
        }

    private:
        Ref<spdlog::logger> GetLogger(bool isCore) { return isCore ? m_CoreLogger : m_ClientLogger; }
        LogRegion           GetRegion(bool isCore) { return isCore ? LogRegion::Core : LogRegion::Client; }
        void                TriggerLogEvent(LogRegion region, LogLevel level, const std::string& msg);

    private:
        Ref<spdlog::logger> m_CoreLogger;
        Ref<spdlog::logger> m_ClientLogger;
    };
} // namespace SnowLeopardEngine