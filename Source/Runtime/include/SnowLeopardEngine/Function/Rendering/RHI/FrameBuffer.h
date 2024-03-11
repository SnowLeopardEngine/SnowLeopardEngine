#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    enum class FrameBufferTextureFormat : uint8_t
    {
        Invalid = 0,

        // Color
        RGBA8,
        RED_INTEGER,

        // Depth/stencil
        DEPTH24,
        DEPTH24_STENCIL8
    };

    struct FrameBufferTextureDesc
    {
        FrameBufferTextureDesc() = default;

        // NOLINTBEGIN
        FrameBufferTextureDesc(FrameBufferTextureFormat format) : TextureFormat(format) {}
        // NOLINTEND

        FrameBufferTextureFormat TextureFormat = FrameBufferTextureFormat::Invalid;
        // TODO: filtering/wrap
    };

    struct FrameBufferAttachmentDesc
    {
        FrameBufferAttachmentDesc() = default;
        FrameBufferAttachmentDesc(std::initializer_list<FrameBufferTextureDesc> attachments) : Attachments(attachments)
        {}

        std::vector<FrameBufferTextureDesc> Attachments;
    };

    struct FrameBufferDesc
    {
        uint32_t                  Width = 0, Height = 0;
        FrameBufferAttachmentDesc AttachmentDesc;
        uint32_t                  Samples = 1;

        bool SwapChainTarget = false;
    };

    class FrameBuffer : public RenderResource
    {
    public:
        virtual ~FrameBuffer() = default;

        virtual void Bind()   = 0;
        virtual void Unbind() = 0;

        virtual void       Resize(uint32_t width, uint32_t height)                  = 0;
        virtual glm::vec4  ReadPixel(uint32_t attachmentIndex, int x, int y)        = 0;
        virtual glm::ivec4 ReadPixelInt(uint32_t attachmentIndex, int x, int y)     = 0;
        virtual int        ReadPixelRedOnly(uint32_t attachmentIndex, int x, int y) = 0;

        virtual void ClearColorAttachment(uint32_t attachmentIndex, const glm::vec4& color) = 0;
        virtual void ClearColorAttachment(uint32_t attachmentIndex, int value)              = 0;

        virtual uint32_t GetColorAttachmentID(uint32_t index) const = 0;

        virtual void BindColorAttachmentTexture(uint32_t index, uint32_t slot) const = 0;
        virtual void BindDepthAttachmentTexture(uint32_t slot) const                 = 0;

        virtual const FrameBufferDesc& GetDesc() const = 0;

        static Ref<FrameBuffer> Create(const FrameBufferDesc& spec);
    };
} // namespace SnowLeopardEngine