#pragma once

#include "SnowLeopardEngine/Function/Animation/AnimationTypeDef.h"

namespace SnowLeopardEngine
{
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

    struct StaticMeshVertexData
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;

        // for picking object using back buffer hack
        int EntityID = -1; 
    };

    struct AnimatedMeshVertexData
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;

        // for picking object using back buffer hack
        int EntityID = -1;

        // attributes for animation
        PerVertexAnimationAttributes AnimationAttributes;
    };

    struct MeshData
    {
        std::vector<StaticMeshVertexData>   StaticVertices;
        std::vector<AnimatedMeshVertexData> AnimatedVertices;
        std::vector<uint32_t>               Indices;

        bool HasAnimationInfo() const
        {
            return !AnimatedVertices.empty();
        }
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

    struct Model
    {
        MeshGroup Meshes;

        // per model animation data
        PerModelAnimationData AnimationData;
    };
} // namespace SnowLeopardEngine