#pragma once

#include "SnowLeopardEngine/Core/Event/Event.h"

namespace SnowLeopardEngine
{
    class WindowMinimizeEvent final : public Event
    {
    public:
        DECLARE_EVENT_TYPE(WindowMinimizeEvent)

        WindowMinimizeEvent() = default;

        virtual const std::string ToString() const override { return "WindowMinimizeEvent"; }
    };

    class WindowResizeEvent final : public Event
    {
    public:
        DECLARE_EVENT_TYPE(WindowResizeEvent)

        WindowResizeEvent(int width, int height) : m_Width(width), m_Height(height) {}

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }

        virtual const std::string ToString() const override
        {
            return std::format("WindowResizeEvent: Width = {0}, Height = {1}", m_Width, m_Height);
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
