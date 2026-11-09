#include "SnowLeopardEngine/Function/Rendering/RenderSystem.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Asset/AssetManager.h"
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
        // Frames without a scene (no active scene or camera) are this colour.
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
        // Bindless: the scene renderer's material textures are one partially bound descriptor array.
        m_Device    = CreateScope<RHI::Device>(ValidationEnabled(), VriFeature_Bindless);
        m_Frame     = CreateScope<RHI::Frame>(*m_Device);

        RHI::Extent extent;
        if (g_EngineContext->WindowSys->IsOffline())
        {
            extent = {g_EngineContext->WindowSys->GetWidth(), g_EngineContext->WindowSys->GetHeight()};
            m_Offscreen = CreateScope<RHI::Texture>(*m_Device, RHI::ColorTextureDesc(extent, VriFormat_RGBA8_UNORM));
        }
        else
        {
            m_Swapchain = CreateScope<RHI::Swapchain>(*m_Device, *g_EngineContext->WindowSys->GetWindow());
            // TODO: configurable VSync. Off, as before.
            m_Swapchain->SetVSync(false);
            extent = m_Swapchain->GetExtent();
        }
        m_Viewport        = {.Offset = {0, 0}, .Extent = {extent.Width, extent.Height}};

        if (const char* captureFrame = std::getenv("SLE_CAPTURE_FRAME"); captureFrame != nullptr)
        {
            m_CaptureFrame          = std::strtoull(captureFrame, nullptr, 10);
            const char* capturePath = std::getenv("SLE_CAPTURE_PATH");
            m_CapturePath           = capturePath != nullptr ? capturePath : "capture.png";
        }

        m_SceneRenderer = CreateScope<SceneRenderer>(*m_Device);
        m_Profiler      = CreateScope<RHI::GpuProfiler>(*m_Device);

        Subscribe(m_LogicSceneLoadedHandler);

        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Initialized (VRI, {0} {1} x {2})",
                               m_Swapchain ? "swapchain" : "offline",
                               extent.Width,
                               extent.Height);
        m_State = SystemState::InitOk;
    }

    RenderSystem::~RenderSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Shutting Down...");

        Unsubscribe(m_LogicSceneLoadedHandler);

        m_Device->WaitIdle();

        // The scene renderer refers to cached textures; the loader caches own them. Free all while the device exists.
        m_FrameGraph.reset();
        m_ViewportTexture.reset();
        m_SceneRenderer.reset();
        m_Profiler.reset();
        IO::ReleaseAllMaterials();
        IO::ReleaseAllTextures();
        AssetManager::Unmount(); // its textures are GPU resources

        m_PendingCapture.reset();
        m_Frame.reset();
        m_Swapchain.reset();
        m_Offscreen.reset();
        m_Device.reset();

        m_State = SystemState::ShutdownOk;
    }

    void RenderSystem::OnTick(float deltaTime)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        // Scene data is collected in Present, right before the frame is recorded (after scripts and physics).
    }

    void RenderSystem::SetRenderToTexture(bool enabled)
    {
        m_RenderToTexture = enabled;
        if (!enabled && m_ViewportTexture)
        {
            m_Device->WaitIdle();
            m_FrameGraph.reset(); // imports the texture
            m_ViewportTexture.reset();
        }
    }

    void RenderSystem::UpdateViewport(const Rect2D& viewport)
    {
        m_Viewport = viewport;
        const RHI::Extent extent {viewport.Extent.Width, viewport.Extent.Height};
        if (!m_RenderToTexture || extent.Empty() || (m_ViewportTexture && m_ViewportTexture->GetExtent() == extent))
        {
            return;
        }
        // The previous frame has completed (one frame in flight), so nothing uses the old texture any more; the graph
        // importing it is rebuilt before its next use.
        m_Device->WaitIdle();
        m_FrameGraph.reset();
        m_ViewportTexture = CreateScope<RHI::Texture>(*m_Device, RHI::ColorTextureDesc(extent, VriFormat_RGBA8_UNORM));
    }

    void RenderSystem::SetVSync(bool enabled)
    {
        if (m_Swapchain)
        {
            m_Swapchain->SetVSync(enabled);
        }
    }

    void RenderSystem::AddOverlay(RenderOverlay* overlay)
    {
        if (std::find(m_Overlays.begin(), m_Overlays.end(), overlay) == m_Overlays.end())
        {
            m_Overlays.push_back(overlay);
            m_FrameGraph.reset(); // the graph has an overlay pass only while there are overlays
        }
    }

    void RenderSystem::RemoveOverlay(RenderOverlay* overlay)
    {
        m_Device->WaitIdle();
        std::erase(m_Overlays, overlay);
        m_FrameGraph.reset();
    }

    void RenderSystem::BuildFrameGraph(RHI::Texture& backbuffer)
    {
        m_FrameGraph       = CreateScope<RenderGraph>(*m_Device);
        m_FrameGraphExtent = backbuffer.GetExtent();
        auto& graph        = *m_FrameGraph;

        // The acquired image's old content is never needed.
        m_Backbuffer = graph.ImportResource("Backbuffer", backbuffer, false);

        m_FrameGraphHasScene     = m_SceneReady;
        m_FrameGraphSceneVersion = m_SceneRenderer->GetGraphVersion();
        m_FrameGraphViewport     = m_RenderToTexture ? m_ViewportTexture.get() : nullptr;

        auto clear = [this, &graph](RenderGraph::Resource resource, const char* name) {
            graph.AddPass(name, {{resource, GraphUsage::ColorWrite, true}}, [this, resource](VriCommandBuffer* cmd, RenderGraph& g) {
                auto& target = g.GetTexture(resource);
                RHI::BeginColorPass(*m_Device, cmd, target.View(), target.GetExtent(), kClearColor);
                m_Device->Core.CmdEndRendering(cmd);
            });
        };

        // The scene goes into the window, or into the viewport texture that the overlays (editor ImGui) sample.
        std::vector<RenderGraph::Use> overlayUses {{m_Backbuffer, GraphUsage::ColorReadWrite}};
        if (m_FrameGraphViewport != nullptr)
        {
            const auto viewport = graph.ImportResource("Viewport", *m_FrameGraphViewport, false);
            if (m_SceneReady)
            {
                m_SceneRenderer->AddPasses(graph, viewport, m_FrameGraphViewport->Desc.format);
            }
            else
            {
                clear(viewport, "ClearViewport");
            }
            clear(m_Backbuffer, "Clear");
            overlayUses.push_back({viewport, GraphUsage::Sampled});
        }
        else if (m_SceneReady)
        {
            m_SceneRenderer->AddPasses(graph, m_Backbuffer, backbuffer.Desc.format);
        }
        else
        {
            clear(m_Backbuffer, "Clear");
        }

        if (!m_Overlays.empty())
        {
            graph.AddPass("Overlays", overlayUses, [this](VriCommandBuffer* cmd, RenderGraph& g) {
                for (auto* overlay : m_Overlays)
                {
                    overlay->Copy(cmd); // transfers stay outside the render pass
                }
                auto& target = g.GetTexture(m_Backbuffer);
                RHI::BeginColorPass(*m_Device, cmd, target.View(), target.GetExtent());
                for (auto* overlay : m_Overlays)
                {
                    overlay->Draw(cmd);
                }
                m_Device->Core.CmdEndRendering(cmd);
            });
        }

        if (m_Swapchain)
        {
            // Presenting is the frame's observable result.
            graph.AddPass("Present", {{m_Backbuffer, GraphUsage::Present}}, [](VriCommandBuffer*, RenderGraph&) {}, true);
        }
        else
        {
            graph.ExportResource(m_Backbuffer); // offline: the image is the result (read by captures)
        }
        graph.Compile();
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

        auto* target = m_Swapchain ? m_Swapchain->Acquire() : m_Offscreen.get();
        if (target == nullptr)
        {
            return; // minimized or out of date
        }

        // The scene renderer stages this frame's data (uploads, instance / material / frame buffers).
        // Rendered at the window's size, or at the editor Viewport's (its texture).
        RHI::Texture* viewport = m_RenderToTexture ? m_ViewportTexture.get() : nullptr;
        m_SceneReady           = false;
        if (!m_RenderToTexture || viewport != nullptr)
        {
            auto scene   = g_EngineContext->SceneMngr->GetActiveScene();
            m_SceneReady = m_SceneRenderer->Prepare(scene.get(),
                                                    viewport != nullptr ? viewport->GetExtent() : target->GetExtent(),
                                                    m_CameraOverride ? &*m_CameraOverride : nullptr);
        }

        if (!m_FrameGraph || m_FrameGraphExtent != target->GetExtent() || m_FrameGraphHasScene != m_SceneReady ||
            m_FrameGraphSceneVersion != m_SceneRenderer->GetGraphVersion() || m_FrameGraphViewport != viewport)
        {
            BuildFrameGraph(*target);
        }
        else
        {
            m_FrameGraph->Bind(m_Backbuffer, *target);
        }

        auto* cmd = m_Frame->Begin();
        m_Profiler->BeginFrame(cmd);
        m_FrameGraph->Execute(cmd, m_Profiler.get()); // ends with the image in the present layout
        m_Profiler->Resolve(cmd);

        ++m_PresentedFrames;
        const bool capture = m_CaptureFrame != 0 && m_PresentedFrames == m_CaptureFrame;
        if (capture)
        {
            Capture(cmd, *target);
        }

        m_Frame->SubmitAndWait();
        m_Profiler->Collect();
        m_SceneRenderer->OnFrameComplete();
        if (m_Swapchain)
        {
            m_Swapchain->Present();
        }

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
        // GPU caches (meshes, materials) of the previous scene refer to its objects.
        m_SceneRenderer->OnSceneChanged();
    }
} // namespace SnowLeopardEngine
