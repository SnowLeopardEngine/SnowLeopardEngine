#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSinglePass.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    void ForwardSinglePass::Draw()
    {
        auto activeScene = g_EngineContext->SceneMngr->GetActiveScene();
        if (activeScene == nullptr)
        {
            return;
        }

        // TODO: for each mesh in the scene draw call.
        SNOW_LEOPARD_CORE_INFO("[Rendering][Forward] ForwardSinglePass is drawing...");
    }
} // namespace SnowLeopardEngine