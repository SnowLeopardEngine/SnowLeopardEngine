#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardPipeline.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    bool ForwardPipeline::Init()
    {
        // TODO: Create and register pipeline states
        SNOW_LEOPARD_CORE_INFO("[Rendering][ForwardPipeline] Registering pipeline states...");

        // TODO: Add render passes
        SNOW_LEOPARD_CORE_INFO("[Rendering][ForwardPipeline] Adding render passes...");

        SNOW_LEOPARD_CORE_INFO("[Rendering][ForwardPipeline] Initialized");

        return true;
    }
} // namespace SnowLeopardEngine