#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    class Event
    {
    public:
        virtual const std::string GetType() const = 0;

        inline bool IsHandled() const { return m_IsHandled; }

        virtual const std::string ToString() const = 0;

    private:
        bool m_IsHandled = false;
    };

#define DECLARE_EVENT_TYPE(eventType) \
    static const std::string GetStaticType() { return #eventType; } \
    const std::string        GetType() const override { return GetStaticType(); }
} // namespace SnowLeopardEngine