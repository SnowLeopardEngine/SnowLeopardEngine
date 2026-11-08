#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Event/EventHandler.h"
#include "SnowLeopardEngine/Core/Event/SceneEvents.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Device.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Resources.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Swapchain.h"
#include "SnowLeopardEngine/Function/Rendering/RenderGraph/RenderGraph.h"
#include "SnowLeopardEngine/Function/Rendering/Renderer/SceneRenderer.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

#include <filesystem>
#include <vector>

namespace SnowLeopardEngine
{
    // Something drawn on top of the frame in the swapchain pass, e.g. the editor's ImGui.
    class RenderOverlay
    {
    public:
        virtual ~RenderOverlay() = default;

        virtual void Upload(RHI::Extent target)    = 0; // stage CPU data; before the swapchain image is acquired
        virtual void Copy(VriCommandBuffer* cmd) = 0; // record transfers; outside the render pass
        virtual void Draw(VriCommandBuffer* cmd) = 0; // record draws; inside the swapchain pass
    };

    // Owns the VRI device, the window's swapchain and the frame. Per frame, Present: overlays and the scene renderer
    // stage their data, the swapchain image is acquired, the frame graph runs (scene passes or a clear, overlays,
    // present), the frame is submitted and presented.
    class RenderSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(RenderSystem)

        void OnTick(float deltaTime);
        void Present();

        void SetVSync(bool enabled);

        // Size of what the scene is rendered at (the window, or the editor's Viewport panel).
        // While rendering to a texture, also (re)creates the viewport texture at this size, so the view handed out by
        // GetOutputTextureID before ImGui records it is the one this frame renders.
        void UpdateViewport(const Rect2D& viewport);

        // Editor Viewport: the scene is rendered into a viewport-sized texture instead of the window; the window gets
        // only the overlays (the editor's ImGui), which sample it. The id is the texture's view (an ImTextureID for
        // the editor's ImGui renderer), 0 until UpdateViewport gave a size. Rows are top to bottom.
        void     SetRenderToTexture(bool enabled);
        uint64_t GetOutputTextureID() const
        {
            return m_ViewportTexture ? reinterpret_cast<uint64_t>(m_ViewportTexture->View()) : 0;
        }

        void AddOverlay(RenderOverlay* overlay);
        void RemoveOverlay(RenderOverlay* overlay);

        RHI::Device&   GetDevice() { return *m_Device; }
        bool           IsOffline() const { return m_Swapchain == nullptr; }
        SceneRenderer* GetSceneRenderer() { return m_SceneRenderer.get(); }

        // Per-pass CPU / GPU times of the last presented frame (GPU from VRI timestamp queries; also sent to Tracy as
        // GPU zones while a Tracy client is connected).
        const std::vector<RHI::GpuProfiler::PassTiming>& GetPassTimings() const { return m_Profiler->GetTimings(); }
        double                                           GetFrameGpuMs() const { return m_Profiler->GetFrameGpuMs(); }
        // Format of the presented image (the swapchain's, or the offline target's).
        VriFormat GetSwapchainFormat() const { return m_Swapchain ? m_Swapchain->GetFormat() : m_Offscreen->Desc.format; }

    private:
        void OnLogicSceneLoaded(const LogicSceneLoadedEvent& e);
        void BuildFrameGraph(RHI::Texture& backbuffer);
        void Capture(VriCommandBuffer* cmd, RHI::Texture& target);

    private:
        Scope<RHI::Device>    m_Device;
        Scope<RHI::Swapchain> m_Swapchain; // null offline
        Scope<RHI::Texture>   m_Offscreen; // offline target, window-sized
        Scope<RHI::Frame>     m_Frame;
        Scope<SceneRenderer>  m_SceneRenderer;
        Scope<RHI::GpuProfiler> m_Profiler;

        std::vector<RenderOverlay*> m_Overlays;

        // Built once per swapchain extent / overlay set; each frame rebinds the acquired swapchain image.
        Scope<RenderGraph>    m_FrameGraph;
        RenderGraph::Resource m_Backbuffer {};
        RHI::Extent           m_FrameGraphExtent {};
        bool                  m_FrameGraphHasScene     = false;
        RHI::Texture*         m_FrameGraphViewport     = nullptr; // viewport texture the graph imports
        uint64_t              m_FrameGraphSceneVersion = 0;
        bool                  m_SceneReady             = false; // the scene renderer prepared this frame

        Rect2D m_Viewport {};
        bool   m_RenderToTexture = false;
        Scope<RHI::Texture> m_ViewportTexture; // render-to-texture target (editor Viewport)

        // Frame capture for regression checks: SLE_CAPTURE_FRAME=<n> writes the n-th presented frame to
        // SLE_CAPTURE_PATH (default: capture.png in the working directory).
        uint64_t                     m_PresentedFrames = 0;
        uint64_t                     m_CaptureFrame    = 0;
        std::filesystem::path        m_CapturePath;
        Scope<RHI::TextureReadback>  m_PendingCapture;

        EventHandler<LogicSceneLoadedEvent> m_LogicSceneLoadedHandler = [this](const LogicSceneLoadedEvent& e) {
            OnLogicSceneLoaded(e);
        };
    };
} // namespace SnowLeopardEngine
