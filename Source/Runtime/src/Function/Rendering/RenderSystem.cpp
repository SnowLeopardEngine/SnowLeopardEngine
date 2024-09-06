#include "SnowLeopardEngine/Function/Rendering/RenderSystem.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

namespace SnowLeopardEngine
{
    RenderSystem::RenderSystem()
    {
        m_Context = CreateRef<GraphicsContext>();
        m_Context->Init();

        // TODO: Configurable vsync
        // Disable VSync
        m_Context->SetVSync(false);

        m_GlobalRenderContext = CreateRef<RenderContext>();
        m_Renderer.Init();

        Subscribe(m_LogicSceneLoadedHandler);

        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    RenderSystem::~RenderSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Shutting Down...");

        Unsubscribe(m_LogicSceneLoadedHandler);

        m_Renderer.Shutdown();
        m_Context->Shutdown();
        m_Context.reset();

        m_State = SystemState::ShutdownOk;
    }

    void RenderSystem::OnTick(float deltaTime)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        // Render
        m_Renderer.RenderFrame(deltaTime);
    }

    void RenderSystem::Present() { m_Context->SwapBuffers(); }

    void RenderSystem::OnLogicSceneLoaded(const LogicSceneLoadedEvent& e)
    {
        // Filter renderables
        m_Renderer.OnLogicSceneLoaded(e.GetLogicScene());
    }
} // namespace SnowLeopardEngine