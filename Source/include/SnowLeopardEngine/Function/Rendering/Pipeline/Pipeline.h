#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderDispatchPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderDrawPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineStateManager.h"

namespace SnowLeopardEngine
{
    class RenderPass;

    struct PipelineInitInfo
    {
        Ref<GraphicsAPI> API;
    };

    class Pipeline
    {
    public:
        virtual ~Pipeline() = default;

        virtual bool Init(const PipelineInitInfo& initInfo)
        {
            m_API          = initInfo.API;
            m_StateManager = PipelineStateManager::Create();
            return true;
        }

        virtual void Tick(float deltaTime);

        virtual void Shutdown() {}

        Ref<GraphicsAPI>          GetAPI() { return m_API; }
        Ref<PipelineStateManager> GetStateManager() { return m_StateManager; }

    protected:
        Ref<GraphicsAPI>                     m_API;
        Ref<PipelineStateManager>            m_StateManager;
        std::vector<Ref<RenderDrawPass>>     m_DrawPasses;
        std::vector<Ref<RenderDispatchPass>> m_DispatchPasses;
    };
} // namespace SnowLeopardEngine