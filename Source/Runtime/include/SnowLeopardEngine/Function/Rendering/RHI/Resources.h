#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Device.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace SnowLeopardEngine::RHI
{
    struct Extent
    {
        uint32_t Width  = 0;
        uint32_t Height = 0;

        bool Empty() const { return Width == 0 || Height == 0; }
        bool operator==(const Extent&) const = default;
    };

    // A VRI texture with whole-resource state tracking. Borrowed textures (swapchain images) are not destroyed.
    class Texture
    {
    public:
        Texture(Device&               device,
                const VriTextureDesc& desc,
                VriTexture*           borrowed = nullptr,
                VriImageAspectFlags   aspect   = VriImageAspect_Color);
        ~Texture();

        Texture(const Texture&)            = delete;
        Texture& operator=(const Texture&) = delete;

        // Records a barrier from the tracked state to `next` (also for equal layouts: write -> read needs one).
        void Transition(VriCommandBuffer* cmd, VriAccessLayoutStage next);

        // Full-resource view, created on first use.
        VriDescriptor* View();

        Extent GetExtent() const { return {Desc.width, Desc.height}; }

        VriTexture*          Handle = nullptr;
        VriTextureDesc       Desc {};
        VriAccessLayoutStage State {};

    private:
        Device&             m_Device;
        bool                m_Owned;
        VriImageAspectFlags m_Aspect;
        VriDescriptor*      m_View = nullptr;
    };

    class Buffer
    {
    public:
        Buffer(Device& device, const VriBufferDesc& desc);
        ~Buffer();

        Buffer(const Buffer&)            = delete;
        Buffer& operator=(const Buffer&) = delete;

        void Transition(VriCommandBuffer* cmd, VriAccessStage next);

        VriBuffer*     Handle = nullptr;
        VriBufferDesc  Desc {};
        VriAccessStage State {};

    private:
        Device& m_Device;
    };

    VriTextureDesc ColorTextureDesc(Extent extent, VriFormat format = VriFormat_RGBA8_UNORM);
    VriTextureDesc DepthTextureDesc(Extent extent);

    // Begins dynamic rendering into one colour view and sets a full-extent viewport and scissor.
    void BeginColorPass(Device&           device,
                        VriCommandBuffer* cmd,
                        VriDescriptor*    view,
                        Extent            extent,
                        const float*      clearColor = nullptr);

    // Blocking uploads (own frame, waits for completion). Rows of `rgba8` are tightly packed.
    Scope<Buffer> UploadBuffer(Device& device, std::span<const std::byte> data, VriBufferUsageFlags usage);
    void          UploadTexture2D(Device& device, Texture& texture, std::span<const std::byte> rgba8);

    // CPU copy of an 8-bit RGBA / BGRA texture as tightly packed RGBA8 rows, recorded into the caller's frame.
    // Read Pixels() only after that frame was submitted and waited for.
    class TextureReadback
    {
    public:
        TextureReadback(Device& device, Texture& texture);
        void                 Record(VriCommandBuffer* cmd, Texture& texture);
        std::vector<uint8_t> Pixels();
        Extent               GetExtent() const { return m_Extent; }

    private:
        Device&   m_Device;
        Buffer    m_Staging;
        Extent    m_Extent;
        VriFormat m_Format;
        uint64_t  m_RowBytes; // staging rows follow the device's buffer <-> texture row alignment
    };
} // namespace SnowLeopardEngine::RHI
