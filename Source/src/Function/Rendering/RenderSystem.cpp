#include "SnowLeopardEngine/Function/Rendering/RenderSystem.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLContext.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    RenderSystem::RenderSystem()
    {
        m_Context = CreateRef<GLContext>();
        m_Context->Init();

        // Currently, set render passes' inputs here

        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    RenderSystem::~RenderSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Shutting Down...");

        m_Context->Shutdown();
        m_Context.reset();
        m_State = SystemState::ShutdownOk;
    }

    void RenderSystem::OnTick(float deltaTime)
    {
        // Currently, define and execute hardcoded render passes.
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. Geometry Pass

        // 2. Lighting Pass
    }

    void RenderSystem::Present() { m_Context->SwapBuffers(); }
} // namespace SnowLeopardEngine