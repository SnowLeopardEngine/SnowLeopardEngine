#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/Resources.h"

#include <vector>

namespace SnowLeopardEngine
{
    class Window;
}

namespace SnowLeopardEngine::RHI
{
    // VRI swapchain of the engine window (ported from libvultra's Swapchain). BGRA8_UNORM: the renderer writes
    // display-encoded colour, and ImGui's vertex colours are sRGB-encoded already.
    class Swapchain
    {
    public:
        Swapchain(Device& device, Window& window, VriFormat format = VriFormat_BGRA8_UNORM);
        ~Swapchain();

        Swapchain(const Swapchain&)            = delete;
        Swapchain& operator=(const Swapchain&) = delete;

        // nullptr while minimized or out of date; try again next frame.
        Texture* Acquire();
        void     Present();
        void     SetVSync(bool enabled); // between frames

        Extent    GetExtent() const { return m_Extent; }
        VriFormat GetFormat() const { return m_Format; }

    private:
        void   Create();
        void   Refresh();
        Extent WindowExtent() const;

        Device&                       m_Device;
        Window&                       m_Window;
        VriSwapChain*                 m_Handle = nullptr;
        VriFormat                     m_Format;
        Extent                        m_Extent {};
        Extent                        m_Requested {};
        bool                          m_Rebuild     = false;
        VriPresentMode                m_PresentMode = VriPresentMode_Fifo;
        std::vector<Scope<Texture>>   m_Images;
    };
} // namespace SnowLeopardEngine::RHI
