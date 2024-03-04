#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/FrameBuffer.h"

namespace SnowLeopardEngine
{
    class GLFrameBuffer : public FrameBuffer
    {
    public:
        explicit GLFrameBuffer(const FrameBufferDesc& spec);
        virtual ~GLFrameBuffer();

        void Invalidate();

        virtual void Bind() override;
        virtual void Unbind() override;

        virtual void       Resize(uint32_t width, uint32_t height) override;
        virtual glm::vec4  ReadPixel(uint32_t attachmentIndex, int x, int y) override;
        virtual glm::ivec4 ReadPixelInt(uint32_t attachmentIndex, int x, int y) override;
        virtual int        ReadPixelRedOnly(uint32_t attachmentIndex, int x, int y) override;

        virtual void ClearColorAttachment(uint32_t attachmentIndex, const glm::vec4& color) override;
        virtual void ClearColorAttachment(uint32_t attachmentIndex, int value) override;

        virtual uint32_t GetColorAttachmentID(uint32_t index) const override
        {
            SNOW_LEOPARD_CORE_ASSERT(index < m_ColorAttachments.size(),
                                     "[GLFrameBuffer] ColorAttachment Index{0} is not attached!");
            return m_ColorAttachments[index];
        }

        virtual void BindColorAttachmentTexture(uint32_t index, uint32_t slot) const override;
        virtual void BindDepthAttachmentTexture(uint32_t slot) const override;

        virtual const FrameBufferDesc& GetDesc() const override { return m_Desc; }

    private:
        uint32_t        m_Name = 0;
        FrameBufferDesc m_Desc;

        std::vector<FrameBufferTextureDesc> m_ColorAttachmentDescs;
        FrameBufferTextureDesc              m_DepthAttachmentDesc = FrameBufferTextureFormat::Invalid;

        std::vector<uint32_t> m_ColorAttachments;
        uint32_t              m_DepthAttachment = 0;

        std::vector<uint32_t> m_ColorAttachmentsToDelete;
        uint32_t              m_DepthAttachmentToDelete = 0;
    };
} // namespace SnowLeopardEngine