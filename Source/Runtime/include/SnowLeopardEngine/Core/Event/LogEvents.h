#pragma once

#include "SnowLeopardEngine/Core/Event/Event.h"
#include "SnowLeopardEngine/Core/Log/LogTypeDef.h"

#include <magic_enum.hpp>

namespace SnowLeopardEngine
{
    class LogEvent : public Event
    {
    public:
        DECLARE_EVENT_TYPE(LogEvent)

        LogEvent(LogRegion region, LogLevel level, const char* msg) : m_Region(region), m_Level(level), m_Message(msg)
        {}

        LogRegion   GetRegion() const { return m_Region; }
        LogLevel    GetLevel() const { return m_Level; }
        const char* GetMessagee() const { return m_Message; }

        virtual const std::string ToString() const override
        {
            return fmt::format("LogEvent, Region = {0}, Level = {1}, Message = {2}",
                               magic_enum::enum_name(m_Region),
                               magic_enum::enum_name(m_Level),
                               m_Message);
        }

    private:
        LogRegion   m_Region;
        LogLevel    m_Level;
        const char* m_Message;
    };
} // namespace SnowLeopardEngine