#include "SnowLeopardEngine/Function/Rendering/RenderSystem.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/IO/MaterialLoader.h"
#include "SnowLeopardEngine/Function/IO/TextureLoader.h"
#include "SnowLeopardEngine/Function/Rendering/Character.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <cstdlib>

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

        bool glyphsLoaded = LoadDefaultFontGlyphs(*m_GlobalRenderContext);
        SNOW_LEOPARD_CORE_ASSERT(glyphsLoaded, "[RenderSystem] Failed to load font glyphs!");

        m_Renderer.Init();

        if (const char* captureFrame = std::getenv("SLE_CAPTURE_FRAME"); captureFrame != nullptr)
        {
            m_CaptureFrame = std::strtoull(captureFrame, nullptr, 10);
            const char* capturePath = std::getenv("SLE_CAPTURE_PATH");
            m_CapturePath           = capturePath != nullptr ? capturePath : "capture.png";
        }

        Subscribe(m_LogicSceneLoadedHandler);

        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    RenderSystem::~RenderSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Shutting Down...");

        Unsubscribe(m_LogicSceneLoadedHandler);

        m_Renderer.Shutdown();
        ReleaseFontGlyphs(*m_GlobalRenderContext);

        // The loader caches own their entries; free them while the GL context still exists.
        IO::ReleaseAllMaterials();
        IO::ReleaseAllTextures(*m_GlobalRenderContext);

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

    void RenderSystem::Present()
    {
        ++m_PresentedFrames;
        if (m_CaptureFrame != 0 && m_PresentedFrames == m_CaptureFrame)
        {
            const bool ok = m_Context->CaptureBackBuffer(m_CapturePath);
            SNOW_LEOPARD_CORE_INFO("[RenderSystem] Capture of frame {0} to {1}: {2}",
                                   m_PresentedFrames,
                                   m_CapturePath.generic_string(),
                                   ok ? "ok" : "failed");
        }

        m_Context->SwapBuffers();
    }

    void RenderSystem::OnLogicSceneLoaded(const LogicSceneLoadedEvent& e)
    {
        // Filter renderables
        m_Renderer.OnLogicSceneLoaded(e.GetLogicScene());
    }
} // namespace SnowLeopardEngine