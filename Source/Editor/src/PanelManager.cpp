#include "SnowLeopardEditor/PanelManager.h"
#include "SnowLeopardEditor/PanelBase.h"
#include "SnowLeopardEditor/Panels/ConsolePanel.h"
#include "SnowLeopardEditor/Panels/InspectorPanel.h"
#include "SnowLeopardEditor/Panels/ResourcePanel.h"
#include "SnowLeopardEditor/Panels/ViewportPanel.h"
#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine::Editor
{
    std::vector<Ref<PanelBase>> PanelManager::s_Panels;

    void PanelManager::Init(const PanelManagerInitInfo& initInfo)
    {
        PanelCommonInitInfo panelInitInfo = {};
        panelInitInfo.ProjectFilePath     = initInfo.ProjectFilePath;

        auto viewportPanel = CreateRef<ViewportPanel>();
        s_Panels.emplace_back(viewportPanel);

        auto consolePanel = CreateRef<ConsolePanel>();
        s_Panels.emplace_back(consolePanel);

        auto inspectorPanel = CreateRef<InspectorPanel>();
        s_Panels.emplace_back(inspectorPanel);

        auto resourcePanel = CreateRef<ResourcePanel>();
        s_Panels.emplace_back(resourcePanel);

        // TODO: add more panels

        for (const auto& panel : s_Panels)
        {
            panel->Init(panelInitInfo);
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