#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"

namespace SnowLeopardEngine
{
    void Pipeline::Tick(float deltaTime)
    {
        // Draw passes
        for (auto& renderPass : m_DrawPasses)
        {
            renderPass->BeginPass();
            renderPass->Draw();
            renderPass->EndPass();
        }
    }
} // namespace SnowLeopardEngine