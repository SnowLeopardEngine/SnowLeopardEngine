#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderPass.h"

namespace SnowLeopardEngine
{
    void Pipeline::Tick(float deltaTime)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        // Draw passes
        for (auto& renderPass : m_DrawPasses)
        {
            renderPass->BeginPass();
            renderPass->Draw();
            renderPass->EndPass();
        }
    }
} // namespace SnowLeopardEngine