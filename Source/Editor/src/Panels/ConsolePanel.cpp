#include "SnowLeopardEditor/Panels/ConsolePanel.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include <fmt/format.h>
#include <magic_enum.hpp>

#define ICON_MDI_MAGNIFY u8"\uF349"

namespace SnowLeopardEngine::Editor
{
    void ConsolePanel::Init()
    {
        m_LogMessagesBox.resize(2100);
        m_LogMessagesEnd = 0;
        m_LogMessagesFilter = static_cast<uint32_t>(LogLevel::Trace) | static_cast<uint32_t>(LogLevel::Info) | 
                    static_cast<uint32_t>(LogLevel::Warn) | static_cast<uint32_t>(LogLevel::Error) | static_cast<uint32_t>(LogLevel::Critical);
        // Register log event
        Subscribe(m_LogEventHandler);
    }

    void ConsolePanel::OnTick(float  /*deltaTime*/)
    {
        // TODO: Yanni Ma, draw the console panel (mainly the messages from your defined container)
        ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);
        ImGui::Begin("console");

        ImGuiStyle& style = ImGui::GetStyle();
        ImGui::AlignTextToFramePadding();
        ImGui::SameLine();
        ImGui::TextUnformatted(ICON_MDI_MAGNIFY);
        ImGui::SameLine();
        float spacing                   = ImGui::GetStyle().ItemSpacing.x;
        ImGui::GetStyle().ItemSpacing.x = 2;
        float levelButtonWidth          = ImGui::CalcTextSize(GetLevelIcon(static_cast<LogLevel>(1))).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float levelButtonWidths         = (levelButtonWidth + ImGui::GetStyle().ItemSpacing.x) * 5;
        {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
            m_Filter.Draw("###ConsoleFilter", ImGui::GetContentRegionAvail().x - (levelButtonWidths));
            auto* drawList = ImGui::GetWindowDrawList();
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            min.x -= 1.0f;
            min.y -= 1.0f;
            max.x += 1.0f;
            max.y += 1.0f;
            if(ImGui::IsItemHovered() && !ImGui::IsItemActive())
            {
                drawList->AddRect(min, max, ImColor(60, 60, 60), 2.0f, 0, 1.5f);
            }
            if(ImGui::IsItemActive())
            {
                drawList->AddRect(min, max, ImColor(80, 80, 80), 2.0f, 0, 1.0f);
            }
        }

        ImGui::SameLine();
        for(int i = 0; i < 5; i++)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::SameLine();
            auto level = static_cast<LogLevel>(i);

            bool levelEnabled = m_LogMessagesFilter & static_cast<uint32_t>(level);
            if(levelEnabled)
            {
                ImU32 imguiColor = ImColor(GetRenderColour(level).r, GetRenderColour(level).g, GetRenderColour(level).b, GetRenderColour(level).a);
                ImGui::PushStyleColor(ImGuiCol_Text, imguiColor);
            }
            else
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5, 0.5f, 0.5f));

            if(ImGui::Button(GetLevelIcon(level)))
            {
                m_LogMessagesFilter ^= static_cast<uint32_t>(level);
            }

            if(ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", GetLevelIcon(level));
            }
            ImGui::PopStyleColor();
        }
        ImGui::GetStyle().ItemSpacing.x = spacing;
        if(!m_Filter.IsActive())
        {
            ImGui::SameLine();
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
            ImGui::SetCursorPosX(ImGui::GetFontSize() * 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ImGui::GetStyle().FramePadding.y));
            ImGui::TextUnformatted("Search...");
        }

        ImGui::Separator();

        if(ImGui::BeginTable("Messages", 3, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg))
        {

            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Type);
            ImGui::TableSetupColumn("Region", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Region);
            ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_NoSort, 0.0f, MyItemColumnID_Message);
            ImGui::TableSetupScrollFreeze(0, 1);

            ImGui::TableHeadersRow();
            // ImGuiUtilities::AlternatingRowsBackground();

            ImGui::TableNextRow();

            for (uint16_t i = 0; i < m_LogMessagesEnd; i = i+3) {
                if(m_LogMessagesFilter & static_cast<uint32_t>(stringToLogLevel(m_LogMessagesBox[i+1]))){
                    ImGui::TableNextColumn();
                    ImU32 iColor = ImColor(GetRenderColour(stringToLogLevel(m_LogMessagesBox[i+1])).r, GetRenderColour(stringToLogLevel(m_LogMessagesBox[i+1])).g, GetRenderColour(stringToLogLevel(m_LogMessagesBox[i+1])).b, GetRenderColour(stringToLogLevel(m_LogMessagesBox[i+1])).a);
                    ImGui::PushStyleColor(ImGuiCol_Text, iColor);
                    const auto *levelIcon = GetLevelIcon(stringToLogLevel(m_LogMessagesBox[i+1]));
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(levelIcon).x - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
                    ImGui::TextUnformatted(levelIcon);

                    ImGui::PopStyleColor();

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(m_LogMessagesBox[i].c_str());

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(m_LogMessagesBox[i+2].c_str());
                    ImGui::TableNextRow();
                }
            }
            ImGui::EndTable();
        }

        ImGui::End();
    }

    void ConsolePanel::Shutdown()
    {
        // Unregister log event
        Unsubscribe(m_LogEventHandler);
    }

    void ConsolePanel::OnLog(const LogEvent& e)
    {
        //std::cout << fmt::format("\t\t\tReceive Log Event: {0}", e.ToString()) << std::endl;
        if(m_LogMessagesEnd >= 2100){
            m_LogMessagesEnd = 0;
        }
        m_LogMessagesBox[m_LogMessagesEnd] = fmt::format("{0}", magic_enum::enum_name(e.GetRegion()));
        m_LogMessagesEnd = m_LogMessagesEnd + 1;
        m_LogMessagesBox[m_LogMessagesEnd] = fmt::format("{0}", magic_enum::enum_name(e.GetLevel()));
        m_LogMessagesEnd = m_LogMessagesEnd + 1;
        m_LogMessagesBox[m_LogMessagesEnd] = fmt::format("{0}", e.GetMessagee());
        m_LogMessagesEnd = m_LogMessagesEnd + 1;
        // TODO: Yanni Ma, use a container to store the log messages.
        // You can get LogLevel and Message from `e`.
        // e.GetRegion() (Core/Client) e.GetLevel() (Trace/Info/Warn/Error/Critical), e.GetMessagee()
    }
} // namespace SnowLeopardEngine::Editor