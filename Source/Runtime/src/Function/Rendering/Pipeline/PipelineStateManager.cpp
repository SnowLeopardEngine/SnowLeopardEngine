#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineStateManager.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLPipelineStateManager.h"

namespace SnowLeopardEngine
{
    bool PipelineStateManager::RegisterState(const Ref<PipelineState>& state)
    {
        if (!InitState(state))
        {
            return false;
        }

        m_States.emplace(state->Name, state);
        return true;
    }

    void PipelineStateManager::UnregisterState(const Ref<PipelineState>& state)
    {
        if (m_States.count(state->Name) > 0)
        {
            m_States.erase(state->Name);
        }

        DestroyState(state);
    }

    const Ref<PipelineState>& PipelineStateManager::GetState(const std::string& name) const
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        const auto& it = m_States.find(name);
        SNOW_LEOPARD_CORE_ASSERT(it != m_States.end(), "[Rendering][Pipeline] Pipeline state {0} not found!", name);

        return it->second;
    }

    void PipelineStateManager::ClearStates()
    {
        for (auto& state : m_States)
        {
            DestroyState(state.second);
        }

        m_States.clear();
    }

    Ref<PipelineStateManager> PipelineStateManager::Create() { return CreateRef<GLPipelineStateManager>(); }
} // namespace SnowLeopardEngine