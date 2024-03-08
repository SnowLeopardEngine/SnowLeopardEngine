#pragma once

namespace SnowLeopardEngine
{
    enum class LogLevel : int
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