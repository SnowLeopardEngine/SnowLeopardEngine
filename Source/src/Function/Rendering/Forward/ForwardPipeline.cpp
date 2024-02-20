#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardPipeline.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSinglePass.h"

namespace SnowLeopardEngine
{
    bool ForwardPipeline::Init()
    {
        // TODO: Create and register pipeline states
        SNOW_LEOPARD_CORE_INFO("[Rendering][ForwardPipeline] Registering pipeline states...");

        // Add forward single pass
        SNOW_LEOPARD_CORE_INFO("[Rendering][ForwardPipeline] Adding ForwardSinglePass...");
        auto forwardSinglePass = CreateRef<ForwardSinglePass>();
        m_DrawPasses.emplace_back(forwardSinglePass);

        SNOW_LEOPARD_CORE_INFO("[Rendering][ForwardPipeline] Initialized");

        return true;
    }
} // namespace SnowLeopardEngine