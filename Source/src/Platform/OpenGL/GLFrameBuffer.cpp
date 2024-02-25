#include "SnowLeopardEngine/Platform/OpenGL/GLFrameBuffer.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    static const uint32_t s_MaxFrameBufferSize = 8192;

    namespace Utils
    {
        static GLenum TextureTarget(bool multisampled)
        {
            return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        }

        static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
        {
            glCreateTextures(TextureTarget(multisampled), count, outID);
        }

        static void AttachColorTexture(uint32_t id,
                                       int      samples,
                                       GLenum   internalFormat,
                                       GLenum   format,
                                       uint32_t width,
                                       uint32_t height,
                                       int      index,
                                       uint32_t fboName)
        {
            bool multisampled = samples > 1;
            if (multisampled)
            {
                glTextureStorage2DMultisample(id, samples, internalFormat, width, height, GL_FALSE);
            }
            else
            {
                glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glTextureStorage2D(id, 1, internalFormat, width, height);
            }

            glNamedFramebufferTexture(fboName, GL_COLOR_ATTACHMENT0 + index, id, 0);
        }

        static void AttachDepthTexture(uint32_t id,
                                       int      samples,
                                       GLenum   format,
                                       GLenum   attachmentType,
                                       uint32_t width,
                                       uint32_t height,
                                       uint32_t fboName)
        {
            bool multisampled = samples > 1;
            if (multisampled)
            {
                glTextureStorage2DMultisample(id, samples, format, width, height, GL_FALSE);
            }
            else
            {
                glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

                GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
                glTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, borderColor);

                glTextureStorage2D(id, 1, format, width, height);
            }
            glNamedFramebufferTexture(fboName, attachmentType, id, 0);
        }

        static bool IsDepthFormat(FrameBufferTextureFormat format)
        {
            return format == FrameBufferTextureFormat::DEPTH24_STENCIL8 || format == FrameBufferTextureFormat::DEPTH24;
        }

        static GLenum FBTextureFormatToGL(FrameBufferTextureFormat format)
        {
            switch (format)
            {
                case FrameBufferTextureFormat::RGBA8:
                    return GL_RGBA8;
                case FrameBufferTextureFormat::RED_INTEGER:
                    return GL_RED_INTEGER;
                case FrameBufferTextureFormat::DEPTH24_STENCIL8:
                    return GL_DEPTH24_STENCIL8;
                case FrameBufferTextureFormat::DEPTH24:
                    return GL_DEPTH_COMPONENT24;
                case FrameBufferTextureFormat::None:
                    SNOW_LEOPARD_CORE_ASSERT(false, "[GLFrameBuffer] Unsupported framebuffer texture format!");
                    break;
            }

            return 0;
        }

    } // namespace Utils

    GLFrameBuffer::GLFrameBuffer(const FrameBufferDesc& spec) : m_Desc(spec)
    {
        for (auto spec : m_Desc.AttachmentDesc.Attachments)
        {
            if (!Utils::IsDepthFormat(spec.TextureFormat))
                m_ColorAttachmentDescs.emplace_back(spec);
            else
                m_DepthAttachmentDesc = spec;
        }

        Invalidate();
    }

    GLFrameBuffer::~GLFrameBuffer()
    {
        glDeleteFramebuffers(1, &m_Name);
        glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
        glDeleteTextures(1, &m_DepthAttachment);
    }

    void GLFrameBuffer::Invalidate()
    {
        if (m_Name)
        {
            glDeleteFramebuffers(1, &m_Name);
            glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
            glDeleteTextures(1, &m_DepthAttachment);

            m_ColorAttachments.clear();
            m_DepthAttachment = 0;
        }

        glCreateFramebuffers(1, &m_Name);

        bool multisample = m_Desc.Samples > 1;

        // Attachments
        if (!m_ColorAttachmentDescs.empty())
        {
            m_ColorAttachments.resize(m_ColorAttachmentDescs.size());
            Utils::CreateTextures(multisample, m_ColorAttachments.data(), m_ColorAttachments.size());

            for (size_t i = 0; i < m_ColorAttachments.size(); ++i)
            {
                switch (m_ColorAttachmentDescs[i].TextureFormat)
                {
                    case FrameBufferTextureFormat::RGBA8:
                        Utils::AttachColorTexture(m_ColorAttachments[i],
                                                  m_Desc.Samples,
                                                  GL_RGBA8,
                                                  GL_RGBA,
                                                  m_Desc.Width,
                                                  m_Desc.Height,
                                                  i,
                                                  m_Name);
                        break;
                    case FrameBufferTextureFormat::RED_INTEGER:
                        Utils::AttachColorTexture(m_ColorAttachments[i],
                                                  m_Desc.Samples,
                                                  GL_R32I,
                                                  GL_RED_INTEGER,
                                                  m_Desc.Width,
                                                  m_Desc.Height,
                                                  i,
                                                  m_Name);
                        break;
                    case FrameBufferTextureFormat::None:
                    case FrameBufferTextureFormat::DEPTH24_STENCIL8:
                    case FrameBufferTextureFormat::DEPTH24:
                        break;
                }
            }
        }

        if (m_DepthAttachmentDesc.TextureFormat != FrameBufferTextureFormat::None)
        {
            Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
            switch (m_DepthAttachmentDesc.TextureFormat)
            {
                case FrameBufferTextureFormat::DEPTH24_STENCIL8:
                    Utils::AttachDepthTexture(m_DepthAttachment,
                                              m_Desc.Samples,
                                              GL_DEPTH24_STENCIL8,
                                              GL_DEPTH_STENCIL_ATTACHMENT,
                                              m_Desc.Width,
                                              m_Desc.Height,
                                              m_Name);
                    break;
                case FrameBufferTextureFormat::DEPTH24:
                    Utils::AttachDepthTexture(m_DepthAttachment,
                                              m_Desc.Samples,
                                              GL_DEPTH_COMPONENT24,
                                              GL_DEPTH_ATTACHMENT,
                                              m_Desc.Width,
                                              m_Desc.Height,
                                              m_Name);
                    break;
                case FrameBufferTextureFormat::None:
                case FrameBufferTextureFormat::RGBA8:
                case FrameBufferTextureFormat::RED_INTEGER:
                    break;
            }
        }

        if (m_ColorAttachments.size() > 1)
        {
            SNOW_LEOPARD_CORE_ASSERT(
                m_ColorAttachments.size() <= 4,
                "[GLFrameBuffer] Currently we only accept 4 color attachments in a single FrameBuffer.");
            GLenum buffers[4] = {
                GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
            glDrawBuffers(m_ColorAttachments.size(), buffers);
        }
        else if (m_ColorAttachments.empty())
        {
            // Only depth-pass
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }

        SNOW_LEOPARD_CORE_ASSERT(glCheckNamedFramebufferStatus(m_Name, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
                                 "[GLFrameBuffer] FrameBuffer is incomplete!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);
    }

    void GLFrameBuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Name);
        glViewport(0, 0, m_Desc.Width, m_Desc.Height);
    }

    void GLFrameBuffer::Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    void GLFrameBuffer::Resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0 || width > s_MaxFrameBufferSize || height > s_MaxFrameBufferSize)
        {
            SNOW_LEOPARD_CORE_WARN("[GLFrameBuffer] Attempted to rezize framebuffer to {0}, {1}", width, height);
            return;
        }
        m_Desc.Width  = width;
        m_Desc.Height = height;

        Invalidate();
    }

    glm::vec4 GLFrameBuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
    {
        SNOW_LEOPARD_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "[GLFrameBuffer] Index out of range!");

        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
        GLubyte pixelData[4];
        glReadPixels(x, y, 1, 1, GL_RGBA, GL_INT, pixelData);
        return glm::vec4(pixelData[0], pixelData[1], pixelData[2], pixelData[3]);
    }

    void GLFrameBuffer::ClearAttachment(uint32_t attachmentIndex, const glm::vec4& color)
    {
        SNOW_LEOPARD_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "[GLFrameBuffer] Index out of range!");

        auto& spec = m_ColorAttachmentDescs[attachmentIndex];

        float newColor[4] = {color.r, color.g, color.b, color.a};
        glClearNamedFramebufferfv(m_Name, GL_COLOR, attachmentIndex, newColor);
    }

    void GLFrameBuffer::BindColorAttachmentTexture(uint32_t index, uint32_t slot) const
    {
        SNOW_LEOPARD_CORE_ASSERT(index < m_ColorAttachments.size(), "[GLFrameBuffer] Index out of range!");
        glBindTextureUnit(slot, m_ColorAttachments[index]);
    }

    void GLFrameBuffer::BindDepthAttachmentTexture(uint32_t slot) const { glBindTextureUnit(slot, m_DepthAttachment); }
} // namespace SnowLeopardEngine