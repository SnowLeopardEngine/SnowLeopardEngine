#include "SnowLeopardEditor/PanelManager.h"
#include "SnowLeopardEditor/Panels/ViewportPanel.h"

namespace SnowLeopardEngine::Editor
{
    std::vector<Ref<PanelBase>> PanelManager::s_Panels;

    void PanelManager::Init()
    {
        auto viewportPanel = CreateRef<ViewportPanel>();
        s_Panels.emplace_back(viewportPanel);

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
} // namespace SnowLeopardEngine::Editor