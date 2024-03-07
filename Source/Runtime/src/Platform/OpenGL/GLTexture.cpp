#include "SnowLeopardEngine/Platform/OpenGL/GLTexture.h"

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

    GLTexture2D::GLTexture2D(const TextureDesc& desc, Buffer* data) : m_Desc(desc)
    {
        m_InternalFormat = PixelColorFormatToInternalFormat(m_Desc.Format);
        m_DataFormat     = PixelColorFormatToDataFormat(m_Desc.Format);

        SNOW_LEOPARD_CORE_ASSERT(m_InternalFormat & m_DataFormat, "[GLTexture2D] Format not supported!");

        glCreateTextures(GL_TEXTURE_2D, 1, &m_Name);

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

        glTextureStorage2D(m_Name, 1, m_InternalFormat, m_Desc.Width, m_Desc.Height);
        glTextureSubImage2D(m_Name, 0, 0, 0, m_Desc.Width, m_Desc.Height, m_DataFormat, GL_UNSIGNED_BYTE, data->Data);

        if (desc.Config.IsGenMipMaps)
        {
            glGenerateTextureMipmap(m_Name);
        }
    }

    GLTexture2D::~GLTexture2D() { glDeleteTextures(1, &m_Name); }

    void GLTexture2D::Bind(uint32_t slot) const { glBindTextureUnit(slot, m_Name); }

    GLCubemap::GLCubemap(const TextureDesc& desc, std::vector<Buffer*> dataList) : m_Desc(desc)
    {
        m_InternalFormat = PixelColorFormatToInternalFormat(m_Desc.Format);
        m_DataFormat     = PixelColorFormatToDataFormat(m_Desc.Format);

        SNOW_LEOPARD_CORE_ASSERT(m_InternalFormat & m_DataFormat, "[GLTexture2D] Format not supported!");

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_Name);

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

        glTextureStorage2D(m_Name, 1, m_InternalFormat, desc.Width, desc.Height);

        for (size_t face = 0; face < 6; ++face)
        {
            glTextureSubImage3D(m_Name,
                                0,
                                0,
                                0,
                                face,
                                desc.Width,
                                desc.Height,
                                1,
                                m_DataFormat,
                                GL_UNSIGNED_BYTE,
                                dataList[face]->Data);
        }

        if (desc.Config.IsGenMipMaps)
        {
            glGenerateTextureMipmap(m_Name);
        }
    }

    GLCubemap::~GLCubemap() { glDeleteTextures(1, &m_Name); }

    void GLCubemap::Bind(uint32_t slot) const { glBindTextureUnit(slot, m_Name); }
} // namespace SnowLeopardEngine