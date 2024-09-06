#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    enum class PixelFormat : GLenum
    {
        Unknown = GL_NONE,

        R8_UNorm = GL_R8,
        R32I     = GL_R32I,

        RGB8_UNorm  = GL_RGB8,
        RGBA8_UNorm = GL_RGBA8,

        RGB8_SNorm  = GL_RGB8_SNORM,
        RGBA8_SNorm = GL_RGBA8_SNORM,

        R16F    = GL_R16F,
        RG16F   = GL_RG16F,
        RGB16F  = GL_RGB16F,
        RGBA16F = GL_RGBA16F,

        RGB32F = GL_RGB32F,

        RGBA32F = GL_RGBA32F,

        RGBA32UI = GL_RGBA32UI,

        Depth16  = GL_DEPTH_COMPONENT16,
        Depth24  = GL_DEPTH_COMPONENT24,
        Depth32F = GL_DEPTH_COMPONENT32F
    };

    enum class TextureType : GLenum
    {
        Texture2D = GL_TEXTURE_2D,
        CubeMap   = GL_TEXTURE_CUBE_MAP
    };

    class Texture
    {
        friend class RenderContext;

    public:
        Texture()               = default;
        Texture(const Texture&) = delete;
        Texture(Texture&&) noexcept;
        ~Texture();

        Texture& operator=(const Texture&) = delete;
        Texture& operator=(Texture&&) noexcept;

        explicit operator bool() const;

        GLenum GetType() const;

        Extent2D    GetExtent() const;
        uint32_t    GetDepth() const;
        uint32_t    GetNumMipLevels() const;
        uint32_t    GetNumLayers() const;
        PixelFormat GetPixelFormat() const;

    private:
        Texture(GLuint id,
                GLenum type,
                PixelFormat,
                Extent2D,
                uint32_t depth,
                uint32_t numMipLevels,
                uint32_t numLayers);

        explicit operator GLuint() const;

    private:
        GLuint m_Id {GL_NONE};
        GLenum m_Type {GL_NONE};

        GLuint m_View {GL_NONE};

        Extent2D m_Extent {0u};
        uint32_t m_Depth {0u};
        uint32_t m_NumMipLevels {1u};
        uint32_t m_NumLayers {0u};

        PixelFormat m_PixelFormat {PixelFormat::Unknown};
    };

    enum class TexelFilter : GLenum
    {
        Nearest = GL_NEAREST,
        Linear  = GL_LINEAR
    };
    enum class MipmapMode : GLenum
    {
        None    = GL_NONE,
        Nearest = GL_NEAREST,
        Linear  = GL_LINEAR
    };

    enum class SamplerAddressMode : GLenum
    {
        Repeat            = GL_REPEAT,
        MirroredRepeat    = GL_MIRRORED_REPEAT,
        ClampToEdge       = GL_CLAMP_TO_EDGE,
        ClampToBorder     = GL_CLAMP_TO_BORDER,
        MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE
    };

    struct SamplerInfo
    {
        TexelFilter MinFilter {TexelFilter::Nearest};
        MipmapMode  MipmapMode {MipmapMode::Linear};
        TexelFilter MagFilter {TexelFilter::Linear};

        SamplerAddressMode AddressModeS {SamplerAddressMode::Repeat};
        SamplerAddressMode AddressModeT {SamplerAddressMode::Repeat};
        SamplerAddressMode AddressModeR {SamplerAddressMode::Repeat};

        float MaxAnisotropy {1.0f};

        std::optional<CompareOp> CompareOperator {};
        glm::vec4                BorderColor {0.0f};
    };

    uint32_t   calcMipLevels(uint32_t size);
    glm::uvec3 calcMipSize(const glm::uvec3& baseSize, uint32_t level);

    const char* toString(PixelFormat);
} // namespace SnowLeopardEngine