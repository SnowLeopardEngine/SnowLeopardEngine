#pragma once

#include "SnowLeopardEngine/Core/Event/Event.h"

namespace SnowLeopardEngine
{
    class WindowResizeEvent final : public Event
    {
    public:
        DECLARE_EVENT_TYPE(WindowResizeEvent)

        WindowResizeEvent(int width, int height) : m_Width(width), m_Height(height) {}

        virtual const std::string ToString() const override
        {
            return fmt::format("WindowResizeEvent: Width = {0}, Height = {1}", m_Width, m_Height);
        }

    private:
        int m_Width;
        int m_Height;
    };

    class WindowCloseEvent final : public Event
    {
    public:
        DECLARE_EVENT_TYPE(WindowCloseEvent)

        virtual const std::string ToString() const override { return "WindowCloseEvent"; }
    };
} // namespace SnowLeopardEngine
