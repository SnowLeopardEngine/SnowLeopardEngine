#pragma once

namespace SnowLeopardEngine
{
    enum class LogLevel
    {
        Trace,
        Info,
        Warn,
        Error,
        Critical
    };

    enum class LogRegion
    {
        Core,
        Client
    };
} // namespace SnowLeopardEngine