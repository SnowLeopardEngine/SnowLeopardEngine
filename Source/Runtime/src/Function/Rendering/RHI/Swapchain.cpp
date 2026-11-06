#include "SnowLeopardEngine/Function/Rendering/RHI/Swapchain.h"
#include "SnowLeopardEngine/Function/Window/Window.h"

#include <vri/integration/vri_sdl3.h>

#include <stdexcept>

namespace SnowLeopardEngine::RHI
{
    Swapchain::Swapchain(Device& device, Window& window, VriFormat format) :
        m_Device(device), m_Window(window), m_Format(format)
    {
        if (format != VriFormat_BGRA8_SRGB && format != VriFormat_BGRA8_UNORM && format != VriFormat_RGBA8_SRGB &&
            format != VriFormat_RGBA8_UNORM)
        {
            throw std::invalid_argument("Swapchain format must be RGBA8 or BGRA8, UNORM or sRGB");
        }
        // Fall back to the other channel order with the same colour encoding.
        if (!(device.Core.GetFormatSupport(device.Handle, m_Format) & VriFormatSupport_ColorAttachment))
        {
            VriFormat alternate = VriFormat_Unknown;
            switch (m_Format)
            {
                case VriFormat_BGRA8_SRGB:
                    alternate = VriFormat_RGBA8_SRGB;
                    break;
                case VriFormat_BGRA8_UNORM:
                    alternate = VriFormat_RGBA8_UNORM;
                    break;
                case VriFormat_RGBA8_SRGB:
                    alternate = VriFormat_BGRA8_SRGB;
                    break;
                case VriFormat_RGBA8_UNORM:
                    alternate = VriFormat_BGRA8_UNORM;
                    break;
                default:
                    break;
            }
            if (!(device.Core.GetFormatSupport(device.Handle, alternate) & VriFormatSupport_ColorAttachment))
            {
                throw std::runtime_error("No RGBA8 / BGRA8 swapchain format with the requested encoding is supported");
            }
            m_Format = alternate;
        }
        Create();
    }

    Swapchain::~Swapchain()
    {
        m_Device.WaitIdle();
        m_Images.clear();
        m_Device.Swap.DestroySwapChain(m_Handle);
    }

    Extent Swapchain::WindowExtent() const
    {
        if (m_Window.IsMinimized())
        {
            return {};
        }
        return {m_Window.GetWidth(), m_Window.GetHeight()};
    }

    void Swapchain::Create()
    {
        m_Requested = WindowExtent();
        VriSwapChainDesc desc {};
        desc.window      = vriWindowHandleFromSDL3(static_cast<SDL_Window*>(m_Window.GetPlatformWindow()));
        desc.queue       = m_Device.Queue;
        desc.format      = m_Format;
        desc.width       = m_Requested.Width;
        desc.height      = m_Requested.Height;
        desc.textureNum  = 2;
        desc.presentMode = m_PresentMode;
        Check(m_Device.Swap.CreateSwapChain(m_Device.Handle, &desc, &m_Handle), "Create swapchain");
        try
        {
            Refresh();
        }
        catch (...)
        {
            m_Device.Swap.DestroySwapChain(m_Handle);
            m_Handle = nullptr;
            throw;
        }
    }

    void Swapchain::Refresh()
    {
        // The surface's real extent can differ from what the window last reported.
        Check(m_Device.Swap.GetSwapChainExtent(m_Handle, &m_Extent.Width, &m_Extent.Height), "Get swapchain extent");
        uint32_t count = 0;
        Check(m_Device.Swap.GetSwapChainTextures(m_Handle, nullptr, &count), "Count swapchain images");
        std::vector<VriTexture*> images(count);
        Check(m_Device.Swap.GetSwapChainTextures(m_Handle, images.data(), &count), "Get swapchain images");
        images.resize(count);
        m_Images.clear();
        for (auto* image : images)
        {
            m_Images.push_back(CreateScope<Texture>(m_Device, ColorTextureDesc(m_Extent, m_Format), image));
        }
    }

    void Swapchain::SetVSync(bool enabled)
    {
        const auto mode = enabled ? VriPresentMode_Fifo : VriPresentMode_Immediate;
        if (m_PresentMode == mode)
        {
            return;
        }
        m_Device.WaitIdle();
        m_Images.clear();
        m_Device.Swap.DestroySwapChain(m_Handle);
        m_Handle      = nullptr;
        m_PresentMode = mode;
        Create();
        m_Rebuild = false;
    }

    Texture* Swapchain::Acquire()
    {
        const auto size = WindowExtent();
        if (size.Empty())
        {
            return nullptr;
        }
        if (m_Rebuild || size != m_Requested)
        {
            m_Device.WaitIdle();
            m_Images.clear(); // views go before Resize destroys the images
            const auto result = m_Device.Swap.Resize(m_Handle, size.Width, size.Height);
            m_Rebuild         = result == VriResult_OutOfDate;
            if (m_Rebuild)
            {
                return nullptr;
            }
            Check(result, "Resize swapchain");
            m_Requested = size;
            Refresh();
        }
        uint32_t   index  = 0;
        const auto result = m_Device.Swap.AcquireNextTexture(m_Handle, nullptr, 0, &index);
        if (result == VriResult_OutOfDate)
        {
            m_Rebuild = true;
            return nullptr;
        }
        Check(result, "Acquire swapchain image");
        return m_Images.at(index).get();
    }

    void Swapchain::Present()
    {
        const auto result = m_Device.Swap.Present(m_Handle, nullptr, 0);
        if (result == VriResult_OutOfDate)
        {
            m_Rebuild = true;
        }
        else
        {
            Check(result, "Present");
        }
    }
} // namespace SnowLeopardEngine::RHI
