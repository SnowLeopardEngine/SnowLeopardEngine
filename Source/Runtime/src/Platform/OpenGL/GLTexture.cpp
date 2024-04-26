#include "SnowLeopardEngine/Platform/OpenGL/GLTexture.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLAPI.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    static GLenum PixelColorFormatToInternalFormat(PixelColorFormat pixelColorFormat)
    {
        switch (pixelColorFormat)
        {
            case PixelColorFormat::RGB8:
                return GL_RGB8;

            case PixelColorFormat::RGBA8:
                return GL_RGBA8;

            case PixelColorFormat::RGB32:
                return GL_RGB32F;

            case PixelColorFormat::RGBA32:
                return GL_RGBA32F;

            default:
                return GL_RGBA8;
        }
    }

    static GLenum PixelColorFormatToDataFormat(PixelColorFormat pixelColorFormat)
    {
        switch (pixelColorFormat)
        {
            case PixelColorFormat::RGB8:
                return GL_RGB;

            case PixelColorFormat::RGBA8:
                return GL_RGBA;

            case PixelColorFormat::RGB32:
                return GL_RGB32F;

            case PixelColorFormat::RGBA32:
                return GL_RGBA32F;

            default:
                return GL_RGBA;
        }
    }

    GLTexture2D::GLTexture2D(const TextureDesc& desc) : m_Desc(desc)
    {
        m_InternalFormat = PixelColorFormatToInternalFormat(m_Desc.Format);
        m_DataFormat     = PixelColorFormatToDataFormat(m_Desc.Format);

        SNOW_LEOPARD_CORE_ASSERT(m_InternalFormat & m_DataFormat, "[GLTexture2D] Format not supported!");

        if (OpenGLAPI::IsDSASupported())
        {
            glCreateTextures(GL_TEXTURE_2D, 1, &m_Name);

            glTextureStorage2D(m_Name, 1, m_InternalFormat, m_Desc.Width, m_Desc.Height);

            switch (desc.Config.WrapMode)
            {
                case TextureWrapMode::Clamp:
                    glTextureParameteri(m_Name, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTextureParameteri(m_Name, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    break;
                case TextureWrapMode::Repeat:
                    glTextureParameteri(m_Name, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTextureParameteri(m_Name, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    break;
                case TextureWrapMode::Mirror:
                    glTextureParameteri(m_Name, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                    glTextureParameteri(m_Name, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                    break;
                case TextureWrapMode::Invalid:
                    // Handle the case when no wrap mode is specified
                    break;
            }

            switch (desc.Config.FilterMode)
            {
                case TextureFilterMode::Point:
                    glTextureParameteri(m_Name, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTextureParameteri(m_Name, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    break;
                case TextureFilterMode::Linear:
                    glTextureParameteri(m_Name, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTextureParameteri(m_Name, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    break;
                case TextureFilterMode::Nearest:
                    glTextureParameteri(m_Name, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                    glTextureParameteri(m_Name, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    break;
                case TextureFilterMode::Invalid:
                    // Handle the case when no filter mode is specified
                    break;
            }

            if (desc.Config.IsGenMipMaps)
            {
                glGenerateTextureMipmap(m_Name);
            }
        }
        else
        {
            glGenTextures(1, &m_Name);
            glBindTexture(GL_TEXTURE_2D, m_Name);

            switch (desc.Config.WrapMode)
            {
                case TextureWrapMode::Clamp:
                    glTexParameteri(m_Name, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(m_Name, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    break;
                case TextureWrapMode::Repeat:
                    glTexParameteri(m_Name, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(m_Name, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    break;
                case TextureWrapMode::Mirror:
                    glTexParameteri(m_Name, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                    glTexParameteri(m_Name, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                    break;
                case TextureWrapMode::Invalid:
                    // Handle the case when no wrap mode is specified
                    break;
            }

            switch (desc.Config.FilterMode)
            {
                case TextureFilterMode::Point:
                    glTexParameteri(m_Name, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(m_Name, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    break;
                case TextureFilterMode::Linear:
                    glTexParameteri(m_Name, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(m_Name, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    break;
                case TextureFilterMode::Nearest:
                    glTexParameteri(m_Name, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                    glTexParameteri(m_Name, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    break;
                case TextureFilterMode::Invalid:
                    // Handle the case when no filter mode is specified
                    break;
            }

            if (desc.Config.IsGenMipMaps)
            {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        }
    }

    GLTexture2D::GLTexture2D(const TextureDesc& desc, Buffer* data) : GLTexture2D(desc)
    {
        if (data != nullptr)
        {
            UploadBuffer(data);
        }
    }

    GLTexture2D::~GLTexture2D() { glDeleteTextures(1, &m_Name); }

    void GLTexture2D::Bind(uint32_t slot) const
    {
        if (OpenGLAPI::IsDSASupported())
        {
            glBindTextureUnit(slot, m_Name);
        }
        else
        {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, m_Name);
        }
    }

    void GLTexture2D::Unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

    void GLTexture2D::UploadBuffer(Buffer* data) const
    {
        if (OpenGLAPI::IsDSASupported())
        {
            glTextureSubImage2D(
                m_Name, 0, 0, 0, m_Desc.Width, m_Desc.Height, m_DataFormat, GL_UNSIGNED_BYTE, data->Data);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         m_InternalFormat,
                         m_Desc.Width,
                         m_Desc.Height,
                         0,
                         m_DataFormat,
                         GL_UNSIGNED_BYTE,
                         data->Data);
        }
    }

    GLCubemap::GLCubemap(const TextureDesc& desc) : m_Desc(desc)
    {
        m_InternalFormat = PixelColorFormatToInternalFormat(m_Desc.Format);
        m_DataFormat     = PixelColorFormatToDataFormat(m_Desc.Format);

        SNOW_LEOPARD_CORE_ASSERT(m_InternalFormat & m_DataFormat, "[GLTexture2D] Format not supported!");

        if (OpenGLAPI::IsDSASupported())
        {
            glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_Name);

            glTextureStorage2D(m_Name, 1, m_InternalFormat, desc.Width, desc.Height);

            glTextureParameteri(m_Name, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_Name, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_Name, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            switch (desc.Config.FilterMode)
            {
                case TextureFilterMode::Point:
                    glTextureParameteri(m_Name, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTextureParameteri(m_Name, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    break;
                case TextureFilterMode::Linear:
                    glTextureParameteri(m_Name, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTextureParameteri(m_Name, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    break;
                case TextureFilterMode::Nearest:
                    glTextureParameteri(m_Name, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                    glTextureParameteri(m_Name, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    break;
                case TextureFilterMode::Invalid:
                    // Handle the case when no filter mode is specified
                    break;
            }

            if (desc.Config.IsGenMipMaps)
            {
                glGenerateTextureMipmap(m_Name);
            }
        }
        else
        {
            glGenTextures(1, &m_Name);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_Name);

            glTexParameteri(m_Name, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(m_Name, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(m_Name, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            switch (desc.Config.FilterMode)
            {
                case TextureFilterMode::Point:
                    glTexParameteri(m_Name, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(m_Name, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    break;
                case TextureFilterMode::Linear:
                    glTexParameteri(m_Name, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(m_Name, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    break;
                case TextureFilterMode::Nearest:
                    glTexParameteri(m_Name, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                    glTexParameteri(m_Name, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    break;
                case TextureFilterMode::Invalid:
                    // Handle the case when no filter mode is specified
                    break;
            }

            if (desc.Config.IsGenMipMaps)
            {
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            }
        }
    }

    GLCubemap::GLCubemap(const TextureDesc& desc, const std::vector<Buffer*>& dataList) : GLCubemap(desc)
    {
        UploadBufferList(dataList);
    }

    GLCubemap::~GLCubemap() { glDeleteTextures(1, &m_Name); }

    void GLCubemap::Bind(uint32_t slot) const
    {
        if (OpenGLAPI::IsDSASupported())
        {
            glBindTextureUnit(slot, m_Name);
        }
        else
        {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_Name);
        }
    }

    void GLCubemap::Unbind() const { glBindTexture(GL_TEXTURE_CUBE_MAP, 0); }

    void GLCubemap::UploadBufferList(const std::vector<Buffer*> dataList) const
    {
        if (OpenGLAPI::IsDSASupported())
        {
            for (size_t face = 0; face < 6; ++face)
            {
                glTextureSubImage3D(m_Name,
                                    0,
                                    0,
                                    0,
                                    face,
                                    m_Desc.Width,
                                    m_Desc.Height,
                                    1,
                                    m_DataFormat,
                                    GL_UNSIGNED_BYTE,
                                    dataList[face]->Data);
            }
        }
        else
        {
            for (size_t face = 0; face < 6; ++face)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                             0,
                             m_InternalFormat,
                             m_Desc.Width,
                             m_Desc.Height,
                             0,
                             m_DataFormat,
                             GL_UNSIGNED_BYTE,
                             dataList[face]->Data);
            }
        }
    }
} // namespace SnowLeopardEngine