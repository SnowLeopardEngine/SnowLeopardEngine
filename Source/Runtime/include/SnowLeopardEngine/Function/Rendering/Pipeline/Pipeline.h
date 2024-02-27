#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderDispatchPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderDrawPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineStateManager.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/FrameBuffer.h"

namespace SnowLeopardEngine
{
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

        void             SetRenderTarget(const Ref<FrameBuffer>& rt) { m_RenderTarget = rt; }
        Ref<FrameBuffer> GetRenderTarget() const { return m_RenderTarget; }

        Ref<GraphicsAPI>          GetAPI() { return m_API; }
        Ref<PipelineStateManager> GetStateManager() { return m_StateManager; }

    protected:
        Ref<GraphicsAPI>                     m_API          = nullptr;
        Ref<PipelineStateManager>            m_StateManager = nullptr;
        Ref<FrameBuffer>                     m_RenderTarget = nullptr;
        std::vector<Ref<RenderDrawPass>>     m_DrawPasses;
        std::vector<Ref<RenderDispatchPass>> m_DispatchPasses;
    };
} // namespace SnowLeopardEngine