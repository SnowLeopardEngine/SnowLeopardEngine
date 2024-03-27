#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Asset/OzzMesh.h"

namespace SnowLeopardEngine
{
    // forward
    class VertexArray;

    struct ViewportDesc
    {
        float X;
        float Y;
        float Width;
        float Height;
    };

    enum class PixelColorFormat : uint8_t
    {
        Invalid = 0,
        RGB8,
        RGBA8,
        RGB32,
        RGBA32
    };

    enum class TextureType : uint8_t
    {
        Invalid = 0,
        Texture2D,
        Texture3D // Cubemap
    };

    enum class TextureWrapMode : uint8_t
    {
        Invalid = 0,
        Clamp,
        Repeat,
        Mirror
    };

    enum class TextureFilterMode : uint8_t
    {
        Invalid = 0,
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

    struct MeshVertexData
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;

        // for picking object using back buffer hack
        int EntityID = -1;
    };

    struct MeshData
    {
        std::vector<MeshVertexData> Vertices;
        std::vector<uint32_t>       Indices;

        Ref<VertexArray> VertexArray = nullptr;
    };

    struct MeshItem
    {
        std::string Name;
        MeshData    Data;

        ozz::sample::Mesh OzzMesh;

        bool Skinned() const { return OzzMesh.skinned(); }
    };

    struct MeshGroup
    {
        std::vector<MeshItem> Items;
    };

    class RenderResource
    {
    public:
        virtual ~RenderResource() = default;
    };
} // namespace SnowLeopardEngine