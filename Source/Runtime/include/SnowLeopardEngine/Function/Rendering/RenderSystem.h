#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Event/EventHandler.h"
#include "SnowLeopardEngine/Core/Event/SceneEvents.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Device.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Resources.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Swapchain.h"
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

    // Owns the VRI device, the window's swapchain and the frame. Per frame: OnTick records the scene (Phase 2.4),
    // Present uploads overlays, acquires the swapchain image, draws the overlays, submits and presents.
    class RenderSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(RenderSystem)

        void OnTick(float deltaTime);
        void Present();

        void SetVSync(bool enabled);

        // Size of what the scene is rendered at (the window, or the editor's Viewport panel).
        void UpdateViewport(const Rect2D& viewport) { m_Viewport = viewport; }

        // Editor Viewport: render the scene into a texture instead of the window. No scene renderer yet (Phase 2.4),
        // so there is no output texture: the id stays 0.
        void     SetRenderToTexture(bool enabled) { m_RenderToTexture = enabled; }
        uint64_t GetOutputTextureID() const { return 0; }

        void AddOverlay(RenderOverlay* overlay);
        void RemoveOverlay(RenderOverlay* overlay);

        RHI::Device& GetDevice() { return *m_Device; }
        VriFormat    GetSwapchainFormat() const { return m_Swapchain->GetFormat(); }

    private:
        void OnLogicSceneLoaded(const LogicSceneLoadedEvent& e);
        void Capture(VriCommandBuffer* cmd, RHI::Texture& target);

    private:
        Scope<RHI::Device>    m_Device;
        Scope<RHI::Swapchain> m_Swapchain;
        Scope<RHI::Frame>     m_Frame;

        std::vector<RenderOverlay*> m_Overlays;

        Rect2D m_Viewport {};
        bool   m_RenderToTexture = false;

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
