#include "SnowLeopardEngine/Function/Rendering/Texture.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include <cstring>

namespace SnowLeopardEngine
{
    Texture::Texture(Texture&& other) noexcept :
        m_Id {other.m_Id}, m_Type {other.m_Type}, m_View {other.m_View}, m_Extent {other.m_Extent},
        m_Depth {other.m_Depth}, m_NumMipLevels {other.m_NumMipLevels}, m_NumLayers {other.m_NumLayers},
        m_PixelFormat {other.m_PixelFormat}
    {
        memset(&other, 0, sizeof(Texture));
    }
    Texture::~Texture()
    {
        if (m_Id != GL_NONE)
            SNOW_LEOPARD_CORE_ERROR("Texture leak: {0}", m_Id);
    }

    Texture& Texture::operator=(Texture&& rhs) noexcept
    {
        if (this != &rhs)
        {
            memcpy(this, &rhs, sizeof(Texture));
            memset(&rhs, 0, sizeof(Texture));
        }
        return *this;
    }

    Texture::operator bool() const { return m_Id != GL_NONE; }

    GLenum Texture::GetType() const { return m_Type; }

    Extent2D    Texture::GetExtent() const { return m_Extent; }
    uint32_t    Texture::GetDepth() const { return m_Depth; }
    uint32_t    Texture::GetNumMipLevels() const { return m_NumMipLevels; }
    uint32_t    Texture::GetNumLayers() const { return m_NumLayers; }
    PixelFormat Texture::GetPixelFormat() const { return m_PixelFormat; }

    Texture::Texture(GLuint      id,
                     GLenum      type,
                     PixelFormat pixelFormat,
                     Extent2D    extent,
                     uint32_t    depth,
                     uint32_t    numMipLevels,
                     uint32_t    numLayers) :
        m_Id {id},
        m_Type {type}, m_Extent {extent}, m_Depth {depth}, m_NumMipLevels {numMipLevels}, m_NumLayers {numLayers},
        m_PixelFormat {pixelFormat}
    {}

    Texture::operator GLuint() const { return m_Id; }

    //
    // Utility:
    //

    uint32_t   calcMipLevels(uint32_t size) { return glm::floor(glm::log2(static_cast<float>(size))) + 1u; }
    glm::uvec3 calcMipSize(const glm::uvec3& baseSize, uint32_t level)
    {
        return glm::vec3 {baseSize} * glm::pow(0.5f, static_cast<float>(level));
    }

    const char* toString(PixelFormat pixelFormat)
    {
        switch (pixelFormat)
        {
            using enum PixelFormat;

            case R8_UNorm:
                return "R8_Unorm";

            case RGB8_UNorm:
                return "RGB8_UNorm";
            case RGBA8_UNorm:
                return "RGBA8_UNorm";

            case RGB8_SNorm:
                return "RGB8_SNorm";
            case RGBA8_SNorm:
                return "RGBA8_SNorm";

            case R16F:
                return "R16F";
            case RG16F:
                return "RG16F";
            case RGB16F:
                return "RGB16F";
            case RGBA16F:
                return "RGBA16F";

            case RGB32F:
                return "RGB32F";

            case RGBA32UI:
                return "RGBA32UI";

            case Depth16:
                return "Depth16";
            case Depth24:
                return "Depth24";
            case Depth32F:
                return "Depth32F";
        }

        return "Undefined";
    }
} // namespace SnowLeopardEngine