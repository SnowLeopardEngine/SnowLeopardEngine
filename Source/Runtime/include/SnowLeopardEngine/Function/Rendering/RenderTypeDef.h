#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"

const uint32_t MaxBoneInfluence = 4;

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
        None = 0,
        RGB8,
        RGBA8,
        RGB32,
        RGBA32
    };

    enum class TextureType : uint8_t
    {
        None = 0,
        Texture2D,
        Texture3D // Cubemap
    };

    enum class TextureWrapMode : uint8_t
    {
        None = 0,
        Clamp,
        Repeat,
        Mirror
    };

    enum class TextureFilterMode : uint8_t
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

        // bone indexes which will influence this vertex
        int BoneIDs[MaxBoneInfluence];

        // weights from each bone
        float Weights[MaxBoneInfluence];
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

    struct BoneInfo
    {
        // id is index in finalBoneMatrices
        int Id = -1;

        // offset matrix transforms vertex from model space to bone space
        glm::mat4 Offset;
    };

    struct AnimationClip
    {};

    struct Model
    {
        MeshGroup Meshes;

        std::vector<AnimationClip>      AnimationClips;
        std::map<std::string, BoneInfo> BoneInfoMap;
        int                             BoneCounter = 0;
    };
} // namespace SnowLeopardEngine