#pragma once

#include "SnowLeopardEditor/PanelBase.h"

namespace SnowLeopardEngine::Editor
{
    // #136: the scene renderer's settings (exposure, shadows, ambient occlusion, bloom, FXAA, selection outline),
    // its statistics and the GPU time of every render-graph pass (RenderSystem::GetPassTimings).
    class RenderSettingsPanel final : public PanelBase
    {
    public:
        virtual void OnTick(float deltaTime) override final;

    private:
        bool  m_SortByTime = false;
        float m_SmoothedGpu = 0.0f;
    };
} // namespace SnowLeopardEngine::Editor
