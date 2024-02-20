#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"

namespace SnowLeopardEngine
{
    struct ViewportDesc
    {
        float X;
        float Y;
        float Width;
        float Height;
    };

    enum class PixelColorFormat
    {
        None = 0,
        RGB8,
        RGBA8,
        RGB32,
        RGBA32
    };

    enum class TextureType
    {
        None = 0,
        Texture2D,
        Texture3D // Cubemap
    };

    enum class TextureWrapMode
    {
        None = 0,
        Clamp,
        Repeat,
        Mirror
    };

    enum class TextureFilterMode
    {
        None = 0,
        Point,
        Linear,
        Nearest
    };

    struct TextureConfig
    {
        TextureWrapMode   WrapMode     = TextureWrapMode::Repeat;
        TextureFilterMode FilterMode   = TextureFilterMode::Linear;
        bool              IsFlip       = true;
        bool              IsGenMipMaps = true;
    };

    struct TextureDesc
    {
        uint32_t         Width  = 1;
        uint32_t         Height = 1;
        PixelColorFormat Format = PixelColorFormat::RGBA8;

        TextureConfig Config;
    };

    struct VertexData
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;
    };

    struct MeshData
    {
        std::vector<VertexData> Vertices;
        std::vector<uint32_t>   Indices;
    };

    struct MeshItem
    {
        std::string Name;
        MeshData    Data;
    };

    struct MeshGroup
    {
        std::vector<MeshItem> Items;
    };
} // namespace SnowLeopardEngine