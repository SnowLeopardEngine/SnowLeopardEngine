#include "SnowLeopardEditor/PanelManager.h"
#include "SnowLeopardEditor/Panels/ConsolePanel.h"
#include "SnowLeopardEditor/Panels/ViewportPanel.h"

namespace SnowLeopardEngine::Editor
{
    std::vector<Ref<PanelBase>> PanelManager::s_Panels;

    void PanelManager::Init()
    {
        auto viewportPanel = CreateRef<ViewportPanel>();
        s_Panels.emplace_back(viewportPanel);

        auto consolePanel = CreateRef<ConsolePanel>();
        s_Panels.emplace_back(consolePanel);

        // TODO: add more panels

        for (const auto& panel : s_Panels)
        {
            panel->Init();
        }
    }

    void PanelManager::OnFixedTick()
    {
        for (auto& panel : s_Panels)
        {
            panel->OnFixedTick();
        }
    }

    void PanelManager::OnTick(float deltaTime)
    {
        for (auto& panel : s_Panels)
        {
            panel->OnTick(deltaTime);
        }
    }

    void PanelManager::Shutdown()
    {
        for (auto& panel : s_Panels)
        {
            panel->Shutdown();
        }
    }
} // namespace SnowLeopardEngine::Editor