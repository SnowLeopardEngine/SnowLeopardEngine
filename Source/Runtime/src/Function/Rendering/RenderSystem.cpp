#include "SnowLeopardEngine/Function/Rendering/RenderSystem.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/IO/MaterialLoader.h"
#include "SnowLeopardEngine/Function/IO/TextureLoader.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <algorithm>
#include <cstdlib>

namespace SnowLeopardEngine
{
    namespace
    {
        // Until the scene renderer exists (Phase 2.4) every frame is this colour.
        constexpr float kClearColor[4] = {0.08f, 0.08f, 0.10f, 1.0f};

        bool ValidationEnabled()
        {
            // SLE_VULKAN_VALIDATION=0 / 1 overrides the build default (on in Debug).
            if (const char* value = std::getenv("SLE_VULKAN_VALIDATION"); value != nullptr)
            {
                return value[0] != '0';
            }
#ifdef NDEBUG
            return false;
#else
            return true;
#endif
        }
    } // namespace

    RenderSystem::RenderSystem()
    {
        m_Device    = CreateScope<RHI::Device>(ValidationEnabled());
        m_Swapchain = CreateScope<RHI::Swapchain>(*m_Device, *g_EngineContext->WindowSys->GetWindow());
        m_Frame     = CreateScope<RHI::Frame>(*m_Device);

        // TODO: configurable VSync. Off, as before.
        m_Swapchain->SetVSync(false);

        const auto extent = m_Swapchain->GetExtent();
        m_Viewport        = {.Offset = {0, 0}, .Extent = {extent.Width, extent.Height}};

        if (const char* captureFrame = std::getenv("SLE_CAPTURE_FRAME"); captureFrame != nullptr)
        {
            m_CaptureFrame          = std::strtoull(captureFrame, nullptr, 10);
            const char* capturePath = std::getenv("SLE_CAPTURE_PATH");
            m_CapturePath           = capturePath != nullptr ? capturePath : "capture.png";
        }

        Subscribe(m_LogicSceneLoadedHandler);

        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Initialized (VRI, swapchain {0} x {1})", extent.Width, extent.Height);
        m_State = SystemState::InitOk;
    }

    RenderSystem::~RenderSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Shutting Down...");

        Unsubscribe(m_LogicSceneLoadedHandler);

        m_Device->WaitIdle();

        // The loader caches own GPU textures; free them while the device exists.
        IO::ReleaseAllMaterials();
        IO::ReleaseAllTextures();

        m_PendingCapture.reset();
        m_Frame.reset();
        m_Swapchain.reset();
        m_Device.reset();

        m_State = SystemState::ShutdownOk;
    }

    void RenderSystem::OnTick(float deltaTime)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        // The scene renderer (render graph + passes) is rebuilt on VRI in Phase 2.2 - 2.4.
    }

    void RenderSystem::SetVSync(bool enabled) { m_Swapchain->SetVSync(enabled); }

    void RenderSystem::AddOverlay(RenderOverlay* overlay)
    {
        if (std::find(m_Overlays.begin(), m_Overlays.end(), overlay) == m_Overlays.end())
        {
            m_Overlays.push_back(overlay);
        }
    }

    void RenderSystem::RemoveOverlay(RenderOverlay* overlay)
    {
        m_Device->WaitIdle();
        std::erase(m_Overlays, overlay);
    }

    void RenderSystem::Present()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        // Overlays stage their geometry before the image is acquired.
        const auto windowExtent = RHI::Extent {g_EngineContext->WindowSys->GetWidth(), g_EngineContext->WindowSys->GetHeight()};
        for (auto* overlay : m_Overlays)
        {
            overlay->Upload(windowExtent);
        }

        auto* target = m_Swapchain->Acquire();
        if (target == nullptr)
        {
            return; // minimized or out of date
        }
        const auto extent = m_Swapchain->GetExtent();

        auto* cmd = m_Frame->Begin();
        for (auto* overlay : m_Overlays)
        {
            overlay->Copy(cmd);
        }

        target->Transition(cmd, {VriAccess_ColorAttachmentWrite, VriLayout_ColorAttachment, VriPipelineStage_ColorAttachmentOutput});
        RHI::BeginColorPass(*m_Device, cmd, target->View(), extent, kClearColor);
        for (auto* overlay : m_Overlays)
        {
            overlay->Draw(cmd);
        }
        m_Device->Core.CmdEndRendering(cmd);

        ++m_PresentedFrames;
        const bool capture = m_CaptureFrame != 0 && m_PresentedFrames == m_CaptureFrame;
        if (capture)
        {
            Capture(cmd, *target);
        }

        target->Transition(cmd, {VriAccess_None, VriLayout_Present, VriPipelineStage_None});
        m_Frame->SubmitAndWait();
        m_Swapchain->Present();

        if (capture && m_PendingCapture)
        {
            const auto captured = m_PendingCapture->GetExtent();
            const auto pixels   = m_PendingCapture->Pixels();
            const bool ok       = stbi_write_png(m_CapturePath.string().c_str(),
                                           int(captured.Width),
                                           int(captured.Height),
                                           4,
                                           pixels.data(),
                                           int(captured.Width) * 4) != 0;
            m_PendingCapture.reset();
            SNOW_LEOPARD_CORE_INFO("[RenderSystem] Capture of frame {0} to {1}: {2}",
                                   m_PresentedFrames,
                                   m_CapturePath.generic_string(),
                                   ok ? "ok" : "failed");
        }
    }

    void RenderSystem::Capture(VriCommandBuffer* cmd, RHI::Texture& target)
    {
        m_PendingCapture = CreateScope<RHI::TextureReadback>(*m_Device, target);
        m_PendingCapture->Record(cmd, target);
    }

    void RenderSystem::OnLogicSceneLoaded(const LogicSceneLoadedEvent& e)
    {
        // Scene GPU resources (meshes, materials, environment maps) are created by the renderer from Phase 2.4.
    }
} // namespace SnowLeopardEngine
