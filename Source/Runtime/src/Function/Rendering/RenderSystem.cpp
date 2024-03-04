#include "SnowLeopardEngine/Function/Rendering/RenderSystem.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"

namespace SnowLeopardEngine
{
    RenderSystem::RenderSystem()
    {
        m_Context = GraphicsContext::Create();
        m_Context->Init();

        m_API = GraphicsAPI::Create(GraphicsBackend::OpenGL);

        PipelineInitInfo pipelineInitInfo = {};
        pipelineInitInfo.API              = m_API;
        m_Pipeline                        = CreateRef<ForwardPipeline>();
        m_Pipeline->Init(pipelineInitInfo);

        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    RenderSystem::~RenderSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Shutting Down...");

        m_Pipeline->Shutdown();

        m_Context->Shutdown();
        m_Context.reset();

        m_State = SystemState::ShutdownOk;
    }

    void RenderSystem::OnTick(float deltaTime)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        // Draw Built-in Deferred Pipeline
        // Now, draw forward instead for testing
        m_Pipeline->Tick(deltaTime);
    }

    void RenderSystem::Present() { m_Context->SwapBuffers(); }
} // namespace SnowLeopardEngine