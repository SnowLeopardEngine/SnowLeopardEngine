#include "SnowLeopardEngine/Function/Rendering/Pass/BaseRenderDrawPass.h"

namespace SnowLeopardEngine
{
    void BaseRenderDrawPass::Draw()
    {
        for (auto& subDrawPass : m_SubDrawPasses)
        {
            subDrawPass->BeginSubPass();
            subDrawPass->Draw();
            subDrawPass->EndSubPass();
        }
    }
} // namespace SnowLeopardEngine