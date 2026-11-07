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
        // View of a single mip level (render target of a mip chain), created on first use.
        VriDescriptor* MipView(uint32_t mip);

        Extent GetExtent() const { return {Desc.width, Desc.height}; }

        VriTexture*          Handle = nullptr;
        VriTextureDesc       Desc {};
        VriAccessLayoutStage State {};

    private:
        Device&             m_Device;
        bool                m_Owned;
        VriImageAspectFlags m_Aspect;
        VriDescriptor*      m_View = nullptr;
        std::vector<VriDescriptor*> m_MipViews;
    };

    class Buffer
    {
    public:
        Buffer(Device& device, const VriBufferDesc& desc);
        ~Buffer();

        Buffer(const Buffer&)            = delete;
        Buffer& operator=(const Buffer&) = delete;

        void Transition(VriCommandBuffer* cmd, VriAccessStage next);

        // Whole-buffer view of `type` (ConstantBuffer / StructuredBuffer / StorageBuffer), created on first use.
        VriDescriptor* View(VriDescriptorType type);

        // Host-visible buffers only (HostUpload): copies `data` to `offset`. With one frame in flight the GPU is
        // not reading the buffer while the next frame is recorded.
        void Write(const void* data, uint64_t size, uint64_t offset = 0);

        VriBuffer*     Handle = nullptr;
        VriBufferDesc  Desc {};
        VriAccessStage State {};

    private:
        Device&                                           m_Device;
        std::vector<std::pair<VriDescriptorType, VriDescriptor*>> m_Views;
    };

    VriTextureDesc ColorTextureDesc(Extent extent, VriFormat format = VriFormat_RGBA8_UNORM);
    VriTextureDesc DepthTextureDesc(Extent extent);

    // Begins dynamic rendering into one colour view and sets a full-extent viewport and scissor.
    void BeginColorPass(Device&           device,
                        VriCommandBuffer* cmd,
                        VriDescriptor*    view,
                        Extent            extent,
                        const float*      clearColor = nullptr);

    // Bytes per texel of the uncompressed colour formats the engine uploads (RGBA8 / RGBA16F / RGBA32F ...).
    uint32_t TexelSize(VriFormat format);

    // Blocking uploads (own frame, waits for completion).
    Scope<Buffer> UploadBuffer(Device& device, std::span<const std::byte> data, VriBufferUsageFlags usage);
    // Writes `data` into a device-local buffer at `offset` (the buffer needs TransferDst usage).
    void UploadBufferRegion(Device& device, Buffer& buffer, uint64_t offset, std::span<const std::byte> data);
    // One tightly packed span per mip level (level 0 first, at most Desc.mipNum); texel size from the format.
    void UploadTexture(Device& device, Texture& texture, std::span<const std::span<const std::byte>> levels);
    void UploadTexture2D(Device& device, Texture& texture, std::span<const std::byte> rgba8); // one RGBA8 level

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
