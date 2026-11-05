#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    enum class PixelFormat : uint8_t
    {
        Unknown = 0,

        R8_UNorm,
        R32I,

        RGB8_UNorm,
        RGBA8_UNorm,

        RGB8_SNorm,
        RGBA8_SNorm,

        R16F,
        RG16F,
        RGB16F,
        RGBA16F,

        RGB32F,

        RGBA32F,

        RGBA32UI,

        Depth16,
        Depth24,
        Depth32F
    };

    enum class TextureType : uint8_t
    {
        Unknown = 0,
        Texture1D,
        Texture1DArray,
        Texture2D,
        Texture2DArray,
        Texture3D,
        CubeMap,
        CubeMapArray
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

        TextureType GetType() const;

        Extent2D    GetExtent() const;
        uint32_t    GetDepth() const;
        uint32_t    GetNumMipLevels() const;
        uint32_t    GetNumLayers() const;
        PixelFormat GetPixelFormat() const;

    private:
        Texture(RenderHandle id,
                TextureType  type,
                PixelFormat,
                Extent2D,
                uint32_t depth,
                uint32_t numMipLevels,
                uint32_t numLayers);

        explicit operator RenderHandle() const;

    private:
        RenderHandle m_Id {kNullHandle};
        TextureType  m_Type {TextureType::Unknown};

        RenderHandle m_View {kNullHandle};

        Extent2D m_Extent {0u};
        uint32_t m_Depth {0u};
        uint32_t m_NumMipLevels {1u};
        uint32_t m_NumLayers {0u};

        PixelFormat m_PixelFormat {PixelFormat::Unknown};
    };

    enum class TexelFilter : uint8_t
    {
        Nearest = 0,
        Linear
    };
    enum class MipmapMode : uint8_t
    {
        None = 0,
        Nearest,
        Linear
    };

    enum class SamplerAddressMode : uint8_t
    {
        Repeat = 0,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder,
        MirrorClampToEdge
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