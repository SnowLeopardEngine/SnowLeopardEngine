#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"

namespace SnowLeopardEngine
{
    enum class FrameBufferTextureFormat : uint8_t
    {
        None = 0,

        // Color
        RGBA8,
        RED_INTEGER,

        // Depth/stencil
        DEPTH24_STENCIL8,

        // Defaults
        Depth = DEPTH24_STENCIL8
    };

    struct FrameBufferTextureDesc
    {
        FrameBufferTextureDesc() = default;

        // NOLINTBEGIN
        FrameBufferTextureDesc(FrameBufferTextureFormat format) : TextureFormat(format) {}
        // NOLINTEND

        FrameBufferTextureFormat TextureFormat = FrameBufferTextureFormat::None;
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
        FrameBufferAttachmentDesc Attachments;
        uint32_t                  Samples = 1;

        bool SwapChainTarget = false;
    };

    class FrameBuffer
    {
    public:
        virtual ~FrameBuffer() = default;

        virtual void UpdateViewport() = 0;

        virtual void      Resize(uint32_t width, uint32_t height)           = 0;
        virtual glm::vec4 ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

        virtual void ClearAttachment(uint32_t attachmentIndex, const glm::vec4& color) = 0;

        virtual uint32_t GetColorAttachmentID(uint32_t index) const = 0;

        virtual const FrameBufferDesc& GetDesc() const = 0;

        static Ref<FrameBuffer> Create(const FrameBufferDesc& spec);
    };
} // namespace SnowLeopardEngine