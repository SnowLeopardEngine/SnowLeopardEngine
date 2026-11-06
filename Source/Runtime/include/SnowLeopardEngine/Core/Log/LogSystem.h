#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Log/LogTypeDef.h"

// clang-format off
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
// clang-format on

#include <magic_enum.hpp>

#include <string_view>
#include <type_traits>

// fmt >= 10 no longer formats enums implicitly. Log enums by name (falling back to the underlying value), which is
// what the engine's log calls expect. An explicit fmt::formatter for a specific enum still takes precedence.
template<typename T>
struct fmt::formatter<T, char, std::enable_if_t<std::is_enum_v<T>>> : fmt::formatter<std::string_view>
{
    auto format(T value, fmt::format_context& ctx) const
    {
        const std::string_view name = magic_enum::enum_name(value);
        if (!name.empty())
        {
            return fmt::formatter<std::string_view>::format(name, ctx);
        }
        return fmt::format_to(ctx.out(), "{}", static_cast<std::underlying_type_t<T>>(value));
    }
};

namespace SnowLeopardEngine
{
    class LogSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(LogSystem)

        template<typename... Args>
        void Message(bool isCore, LogLevel level, std::string_view fmt, Args&&... args)
        {
            switch (level)
            {
                case LogLevel::Trace:
                    Trace(isCore, fmt, args...);
                    break;

                case LogLevel::Info:
                    Info(isCore, fmt, args...);
                    break;

                case LogLevel::Warn:
                    Warn(isCore, fmt, args...);
                    break;

                case LogLevel::Error:
                    Error(isCore, fmt, args...);
                    break;

                case LogLevel::Critical:
                    Critical(isCore, fmt, args...);
                    break;
            }
        }

        template<typename... Args>
        void Trace(bool isCore, std::string_view fmt, Args&&... args)
        {
            auto formatMsg = GetFormatString(fmt, args...);
            GetLogger(isCore)->trace(formatMsg);
            TriggerLogEvent(GetRegion(isCore), LogLevel::Trace, formatMsg);
        }

        template<typename... Args>
        void Info(bool isCore, std::string_view fmt, Args&&... args)
        {
            auto formatMsg = GetFormatString(fmt, args...);
            GetLogger(isCore)->info(formatMsg);
            TriggerLogEvent(GetRegion(isCore), LogLevel::Info, formatMsg);
        }

        template<typename... Args>
        void Warn(bool isCore, std::string_view fmt, Args&&... args)
        {
            auto formatMsg = GetFormatString(fmt, args...);
            GetLogger(isCore)->warn(formatMsg);
            TriggerLogEvent(GetRegion(isCore), LogLevel::Warn, formatMsg);
        }

        template<typename... Args>
        void Error(bool isCore, std::string_view fmt, Args&&... args)
        {
            auto formatMsg = GetFormatString(fmt, args...);
            GetLogger(isCore)->error(formatMsg);
            TriggerLogEvent(GetRegion(isCore), LogLevel::Error, formatMsg);
        }

        template<typename... Args>
        void Critical(bool isCore, std::string_view fmt, Args&&... args)
        {
            auto formatMsg = GetFormatString(fmt, args...);
            GetLogger(isCore)->critical(formatMsg);
            TriggerLogEvent(GetRegion(isCore), LogLevel::Critical, formatMsg);
        }

    private:
        template<typename... Args>
        std::string GetFormatString(std::string_view fmt, Args&&... args)
        {
            return fmt::vformat(fmt, fmt::make_format_args(args...));
        }

        Ref<spdlog::logger> GetLogger(bool isCore) { return isCore ? m_CoreLogger : m_ClientLogger; }
        LogRegion           GetRegion(bool isCore) { return isCore ? LogRegion::Core : LogRegion::Client; }
        void                TriggerLogEvent(LogRegion region, LogLevel level, std::string_view msg);

    private:
        Ref<spdlog::logger> m_CoreLogger;
        Ref<spdlog::logger> m_ClientLogger;
    };
} // namespace SnowLeopardEngine