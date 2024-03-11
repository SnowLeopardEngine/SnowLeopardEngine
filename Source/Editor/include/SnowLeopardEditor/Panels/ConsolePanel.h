#pragma once

#include "SnowLeopardEditor/PanelBase.h"
#include "SnowLeopardEngine/Core/Event/EventHandler.h"
#include "SnowLeopardEngine/Core/Event/LogEvents.h"

#include <IconsMaterialDesignIcons.h>
#include <glm/glm.hpp>
#include <imgui.h>

namespace SnowLeopardEngine::Editor
{
    class ConsolePanel : public PanelBase
    {
    public:
        virtual void Init() override final;
        virtual void OnTick(float deltaTime) override final;
        virtual void Shutdown() override final;

    private:
        void OnLog(const LogEvent& e);

        EventHandler<LogEvent> m_LogEventHandler = [this](const LogEvent& e) { OnLog(e); };

        std::vector<std::string> m_LogMessagesBox;
        uint16_t                 m_LogMessagesEnd;
        uint32_t                 m_LogMessagesFilter;
        ImGuiTextFilter          m_Filter;
        bool m_AllowToBottom;
        bool m_RequestToBottom;

        enum MyItemColumnID
        {
            MyItemColumnID_Region,
            MyItemColumnID_Message,
            MyItemColumnID_Type
        };

        static LogLevel stringToLogLevel(const std::string& levelStr)
        {
            auto levelOpt = magic_enum::enum_cast<LogLevel>(levelStr);
            return levelOpt.value_or(LogLevel::Info);
        }

        static const char* GetLevelIcon(LogLevel level)
        {
            switch (level)
            {
                case LogLevel::Trace:
                    return ICON_MDI_MESSAGE_TEXT;
                case LogLevel::Info:
                    return ICON_MDI_INFORMATION;
                case LogLevel::Warn:
                    return ICON_MDI_ALERT;
                case LogLevel::Error:
                    return ICON_MDI_CLOSE_OCTAGON;
                case LogLevel::Critical:
                    return ICON_MDI_ALERT_OCTAGRAM;
                default:
                    return "Unknown name";
            }
        }

        static glm::vec4 GetRenderColour(LogLevel level)
        {
            switch (level)
            {
                case LogLevel::Trace:
                    return {0.75f, 0.75f, 0.75f, 1.00f}; // Gray
                case LogLevel::Info:
                    return {0.40f, 0.70f, 1.00f, 1.00f}; // Blue
                case LogLevel::Warn:
                    return {1.00f, 1.00f, 0.00f, 1.00f}; // Yellow
                case LogLevel::Error:
                    return {1.00f, 0.25f, 0.25f, 1.00f}; // Red
                case LogLevel::Critical:
                    return {0.6f, 0.2f, 0.8f, 1.00f}; // Purple
                default:
                    return {1.00f, 1.00f, 1.00f, 1.00f};
            }
        }
    };
} // namespace SnowLeopardEngine::Editor