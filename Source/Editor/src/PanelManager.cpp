#include "SnowLeopardEditor/PanelManager.h"
#include "SnowLeopardEditor/PanelBase.h"
#include "SnowLeopardEditor/Panels/ConsolePanel.h"
#include "SnowLeopardEditor/Panels/HierarchyPanel.h"
#include "SnowLeopardEditor/Panels/InspectorPanel.h"
#include "SnowLeopardEditor/Panels/RenderSettingsPanel.h"
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

        s_Panels.emplace_back(CreateRef<HierarchyPanel>());

        auto consolePanel = CreateRef<ConsolePanel>();
        s_Panels.emplace_back(consolePanel);

        auto inspectorPanel = CreateRef<InspectorPanel>();
        s_Panels.emplace_back(inspectorPanel);

        auto resourcePanel = CreateRef<ResourcePanel>();
        s_Panels.emplace_back(resourcePanel);

        // After the Inspector: of the panels docked together, the first one submitted is the front tab.
        s_Panels.emplace_back(CreateRef<RenderSettingsPanel>());


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
        // Panels hold scenes (the Viewport's editing scene); release them while the engine is still up, not at
        // static destruction.
        s_Panels.clear();
    }
} // namespace SnowLeopardEngine::Editor