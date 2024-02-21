#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardPipeline.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSinglePass.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"

namespace SnowLeopardEngine
{
    bool ForwardPipeline::Init(const PipelineInitInfo& initInfo)
    {
        Pipeline::Init(initInfo);

        // Create and register pipeline states
        SNOW_LEOPARD_CORE_INFO("[Rendering][ForwardPipeline] Registering pipeline states...");
        auto forward                = CreateRef<PipelineState>();
        forward->Name               = "Forward";
        forward->VertexShaderName   = "Phong.vert";
        forward->FragmentShaderName = "Phong.frag";
        m_StateManager->RegisterState(forward);

        // Add forward single pass
        SNOW_LEOPARD_CORE_INFO("[Rendering][ForwardPipeline] Adding ForwardSinglePass...");
        auto forwardSinglePass = CreateRef<ForwardSinglePass>();
        forwardSinglePass->BindPipeline(this);
        m_DrawPasses.emplace_back(forwardSinglePass);

        SNOW_LEOPARD_CORE_INFO("[Rendering][ForwardPipeline] Initialized");

        return true;
    }
} // namespace SnowLeopardEngine