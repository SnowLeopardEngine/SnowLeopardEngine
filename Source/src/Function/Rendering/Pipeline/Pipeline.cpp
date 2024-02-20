#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderPass.h"

namespace SnowLeopardEngine
{
    void Pipeline::Tick(float deltaTime)
    {
        // Draw passes
        for (auto& renderPass : m_RenderPasses)
        {
            renderPass->BeginPass();
            renderPass->Draw();
            renderPass->EndPass();
        }
    }

    bool Pipeline::RegisterState(PipelineState& state)
    {
        // TODO: Init state and check

        m_States.emplace(state.Name, &state);
        return true;
    }

    void Pipeline::UnregisterState(PipelineState& state)
    {
        if (m_States.count(state.Name) > 0)
        {
            m_States.erase(state.Name);
        }

        // TODO: Destroy state
    }

    const Ref<PipelineState>& Pipeline::GetState(const std::string& name) const
    {
        const auto& it = m_States.find(name);
        SNOW_LEOPARD_CORE_ASSERT(it != m_States.end(), "[Rendering][Pipeline] Pipeline state {0} not found!", name);

        return it->second;
    }

    void Pipeline::ClearStates()
    {
        // TODO: Destroy states

        m_States.clear();
    }
} // namespace SnowLeopardEngine