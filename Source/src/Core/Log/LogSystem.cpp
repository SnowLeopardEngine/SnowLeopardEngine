#include "SnowLeopardEngine/Core/Log/LogSystem.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace SnowLeopardEngine
{
    LogSystem::LogSystem()
    {
        std::vector<spdlog::sink_ptr> logSinks;

        logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("SnowLeopardEngine.log", true));
        logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

        logSinks[0]->set_pattern("%^[%T] %n: %v%$");
        logSinks[1]->set_pattern("[%T] [%l] %n: %v");

        m_CoreLogger = std::make_shared<spdlog::logger>("SNOW_LEOPARD_ENGINE_CORE", begin(logSinks), end(logSinks));
        spdlog::register_logger(m_CoreLogger);
        m_CoreLogger->set_level(spdlog::level::trace);
        m_CoreLogger->flush_on(spdlog::level::trace);

        m_ClientLogger = std::make_shared<spdlog::logger>("SNOW_LEOPARD_ENGINE_APP ", begin(logSinks), end(logSinks));
        spdlog::register_logger(m_ClientLogger);
        m_ClientLogger->set_level(spdlog::level::trace);
        m_ClientLogger->flush_on(spdlog::level::trace);

        m_State = SystemState::InitOk;
    }

    LogSystem::~LogSystem()
    {
        spdlog::shutdown();
        m_State = SystemState::ShutdownOk;
    }
} // namespace SnowLeopardEngine
