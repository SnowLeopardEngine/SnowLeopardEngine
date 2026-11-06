#include "SnowLeopardEngine/Function/Rendering/RHI/Resources.h"
#include "SnowLeopardEngine/Core/Base/Base.h"

#include <algorithm>
#include <cstring>
#include <numeric>
#include <stdexcept>

namespace SnowLeopardEngine::RHI
{
    Texture::Texture(Device& device, const VriTextureDesc& desc, VriTexture* borrowed, VriImageAspectFlags aspect) :
        Handle(borrowed), Desc(desc), m_Device(device), m_Owned(borrowed == nullptr), m_Aspect(aspect)
    {
        if (m_Owned)
        {
            Check(device.Core.CreateTexture(device.Handle, &Desc, &Handle), "Create texture");
        }
    }

    Texture::~Texture()
    {
        if (m_View != nullptr)
        {
            m_Device.Core.DestroyDescriptor(m_View);
        }
        if (m_Owned)
        {
            m_Device.Core.DestroyTexture(Handle);
        }
    }

    void Texture::Transition(VriCommandBuffer* cmd, VriAccessLayoutStage next)
    {
        VriTextureBarrierDesc barrier {};
        barrier.texture = Handle;
        barrier.before  = State;
        barrier.after   = next;
        barrier.aspect  = m_Aspect;
        VriBarrierGroupDesc group {};
        group.textures   = &barrier;
        group.textureNum = 1;
        m_Device.Core.CmdBarrier(cmd, &group);
        State = next;
    }

    VriDescriptor* Texture::View()
    {
        if (m_View == nullptr)
        {
            VriTextureViewDesc view {};
            view.texture = Handle;
            switch (Desc.type)
            {
                case VriTextureType_2DArray:
                    view.viewType = VriTextureViewType_2DArray;
                    break;
                case VriTextureType_3D:
                    view.viewType = VriTextureViewType_3D;
                    break;
                case VriTextureType_Cube:
                    view.viewType = VriTextureViewType_Cube;
                    break;
                case VriTextureType_CubeArray:
                    view.viewType = VriTextureViewType_CubeArray;
                    break;
                default:
                    view.viewType = VriTextureViewType_2D;
                    break;
            }
            view.format   = Desc.format;
            view.aspect   = m_Aspect;
            view.layerNum = Desc.layerNum;
            Check(m_Device.Core.CreateTextureView(m_Device.Handle, &view, &m_View), "Create texture view");
        }
        return m_View;
    }

    Buffer::Buffer(Device& device, const VriBufferDesc& desc) : Desc(desc), m_Device(device)
    {
        Check(device.Core.CreateBuffer(device.Handle, &Desc, &Handle), "Create buffer");
    }

    Buffer::~Buffer() { m_Device.Core.DestroyBuffer(Handle); }

    void Buffer::Transition(VriCommandBuffer* cmd, VriAccessStage next)
    {
        VriBufferBarrierDesc barrier {Handle, State, next};
        VriBarrierGroupDesc  group {};
        group.buffers   = &barrier;
        group.bufferNum = 1;
        m_Device.Core.CmdBarrier(cmd, &group);
        State = next;
    }

    VriTextureDesc ColorTextureDesc(Extent extent, VriFormat format)
    {
        VriTextureDesc desc {};
        desc.type      = VriTextureType_2D;
        desc.format    = format;
        desc.width     = extent.Width;
        desc.height    = extent.Height;
        desc.depth     = 1;
        desc.mipNum    = 1;
        desc.layerNum  = 1;
        desc.sampleNum = 1;
        desc.usage     = VriTextureUsage_ColorAttachment | VriTextureUsage_ShaderResource | VriTextureUsage_TransferSrc |
                     VriTextureUsage_TransferDst;
        desc.memoryLocation = VriMemoryLocation_Device;
        return desc;
    }

    VriTextureDesc DepthTextureDesc(Extent extent)
    {
        auto desc                          = ColorTextureDesc(extent, VriFormat_D32_SFLOAT);
        desc.usage                         = VriTextureUsage_DepthStencilAttachment | VriTextureUsage_ShaderResource;
        desc.clearValue.depthStencil.depth = 1;
        return desc;
    }

    void BeginColorPass(Device& device, VriCommandBuffer* cmd, VriDescriptor* view, Extent extent, const float* clearColor)
    {
        VriAttachmentDesc color {};
        color.view    = view;
        color.loadOp  = clearColor != nullptr ? VriAttachmentLoadOp_Clear : VriAttachmentLoadOp_Load;
        color.storeOp = VriAttachmentStoreOp_Store;
        if (clearColor != nullptr)
        {
            std::copy_n(clearColor, 4, color.clearValue.color.f32);
        }
        VriAttachmentsDesc attachments {};
        attachments.colors     = &color;
        attachments.colorNum   = 1;
        attachments.renderArea = {0, 0, extent.Width, extent.Height};
        attachments.layerNum   = 1;
        device.Core.CmdBeginRendering(cmd, &attachments);
        VriViewport viewport {0, 0, float(extent.Width), float(extent.Height), 0, 1};
        VriRect     scissor {0, 0, extent.Width, extent.Height};
        device.Core.CmdSetViewports(cmd, &viewport, 1);
        device.Core.CmdSetScissors(cmd, &scissor, 1);
    }

    Scope<Buffer> UploadBuffer(Device& device, std::span<const std::byte> data, VriBufferUsageFlags usage)
    {
        if (data.empty())
        {
            throw std::invalid_argument("Cannot upload an empty buffer");
        }
        Buffer staging(device, {data.size_bytes(), 0, VriBufferUsage_TransferSrc, VriMemoryLocation_HostUpload});
        void*  mapped = device.Core.MapBuffer(staging.Handle, 0, data.size_bytes());
        if (mapped == nullptr)
        {
            throw std::runtime_error("Map staging buffer failed");
        }
        std::memcpy(mapped, data.data(), data.size_bytes());
        device.Core.UnmapBuffer(staging.Handle);

        auto result = CreateScope<Buffer>(
            device,
            VriBufferDesc {data.size_bytes(), 0, usage | VriBufferUsage_TransferDst, VriMemoryLocation_Device});
        Frame frame(device);
        auto* cmd = frame.Begin();
        result->Transition(cmd, {VriAccess_CopyDestinationWrite, VriPipelineStage_Transfer});
        VriBufferCopyDesc copy {};
        copy.size = data.size_bytes();
        device.Core.CmdCopyBuffer(cmd, result->Handle, staging.Handle, &copy);
        result->Transition(cmd, {VriAccess_ShaderResourceRead, VriPipelineStage_AllCommands});
        frame.SubmitAndWait();
        return result;
    }

    void UploadTexture2D(Device& device, Texture& texture, std::span<const std::byte> rgba8)
    {
        const uint32_t width  = texture.Desc.width;
        const uint32_t height = texture.Desc.height;
        if (rgba8.size_bytes() < uint64_t(width) * height * 4)
        {
            throw std::invalid_argument("Texture upload data is smaller than the texture");
        }
        // Rows of the staging buffer follow the device's buffer -> texture row alignment.
        const uint32_t alignment =
            std::lcm(256u, std::max(1u, device.Core.GetDeviceDesc(device.Handle)->uploadBufferTextureRowAlignment));
        const uint32_t pitch = (width * 4 + alignment - 1) / alignment * alignment;
        const uint64_t bytes = uint64_t(pitch) * height;

        Buffer staging(device, {bytes, 0, VriBufferUsage_TransferSrc, VriMemoryLocation_HostUpload});
        auto*  mapped = static_cast<std::byte*>(device.Core.MapBuffer(staging.Handle, 0, bytes));
        if (mapped == nullptr)
        {
            throw std::runtime_error("Map texture staging buffer failed");
        }
        for (uint32_t row = 0; row < height; ++row)
        {
            std::memcpy(mapped + uint64_t(row) * pitch, rgba8.data() + uint64_t(row) * width * 4, width * 4);
        }
        device.Core.UnmapBuffer(staging.Handle);

        Frame frame(device);
        auto* cmd = frame.Begin();
        texture.Transition(cmd, {VriAccess_CopyDestinationWrite, VriLayout_CopyDestination, VriPipelineStage_Transfer});
        VriBufferTextureCopyDesc copy {};
        copy.bufferRowLength   = pitch / 4;
        copy.bufferImageHeight = height;
        copy.texture           = {0, 0, 1, VriImageAspect_Color, 0, 0, 0, width, height, 1};
        device.Core.CmdUploadBufferToTexture(cmd, texture.Handle, staging.Handle, &copy);
        texture.Transition(cmd, {VriAccess_ShaderResourceRead, VriLayout_ShaderResource, VriPipelineStage_AllCommands});
        frame.SubmitAndWait();
    }

    namespace
    {
        uint64_t ReadbackRowBytes(Device& device, const Texture& texture)
        {
            const uint64_t alignment = std::lcm(
                uint64_t(4), uint64_t(std::max(device.Core.GetDeviceDesc(device.Handle)->uploadBufferTextureRowAlignment, 1u)));
            const uint64_t tight = uint64_t(texture.Desc.width) * 4;
            return (tight + alignment - 1) / alignment * alignment;
        }

        VriBufferDesc ReadbackBufferDesc(Device& device, const Texture& texture)
        {
            switch (texture.Desc.format)
            {
                case VriFormat_RGBA8_UNORM:
                case VriFormat_RGBA8_SRGB:
                case VriFormat_BGRA8_UNORM:
                case VriFormat_BGRA8_SRGB:
                    break;
                default:
                    throw std::invalid_argument("Readback supports 8-bit RGBA / BGRA textures only");
            }
            const uint64_t bytes = ReadbackRowBytes(device, texture) * texture.Desc.height;
            return {bytes, 0, VriBufferUsage_TransferDst, VriMemoryLocation_HostReadback};
        }
    } // namespace

    TextureReadback::TextureReadback(Device& device, Texture& texture) :
        m_Device(device), m_Staging(device, ReadbackBufferDesc(device, texture)), m_Extent(texture.GetExtent()),
        m_Format(texture.Desc.format), m_RowBytes(ReadbackRowBytes(device, texture))
    {}

    void TextureReadback::Record(VriCommandBuffer* cmd, Texture& texture)
    {
        const auto previous = texture.State;
        texture.Transition(cmd, {VriAccess_CopySourceRead, VriLayout_CopySource, VriPipelineStage_Transfer});
        VriBufferTextureCopyDesc region {};
        region.bufferRowLength  = uint32_t(m_RowBytes / 4);
        region.texture.layerNum = 1;
        region.texture.aspect   = VriImageAspect_Color;
        region.texture.width    = m_Extent.Width;
        region.texture.height   = m_Extent.Height;
        region.texture.depth    = 1;
        m_Device.Core.CmdReadbackTextureToBuffer(cmd, m_Staging.Handle, texture.Handle, &region);
        texture.Transition(cmd, previous);
    }

    std::vector<uint8_t> TextureReadback::Pixels()
    {
        const uint64_t bytes = m_RowBytes * m_Extent.Height;
        const auto*    data  = static_cast<const uint8_t*>(m_Device.Core.MapBuffer(m_Staging.Handle, 0, bytes));
        if (data == nullptr)
        {
            throw std::runtime_error("Map readback buffer failed");
        }
        const size_t         tight = size_t(m_Extent.Width) * 4;
        std::vector<uint8_t> pixels(tight * m_Extent.Height);
        for (uint32_t row = 0; row < m_Extent.Height; ++row)
        {
            std::memcpy(pixels.data() + row * tight, data + row * m_RowBytes, tight);
        }
        m_Device.Core.UnmapBuffer(m_Staging.Handle);

        if (m_Format == VriFormat_BGRA8_UNORM || m_Format == VriFormat_BGRA8_SRGB)
        {
            for (size_t i = 0; i < pixels.size(); i += 4)
            {
                std::swap(pixels[i], pixels[i + 2]);
            }
        }
        return pixels;
    }
} // namespace SnowLeopardEngine::RHI
