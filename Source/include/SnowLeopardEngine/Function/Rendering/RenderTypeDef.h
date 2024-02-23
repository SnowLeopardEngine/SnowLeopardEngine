#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"

#define NUM_BONES_PER_VERTEX 4

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

    struct BoneData
    {
        unsigned int IDs[NUM_BONES_PER_VERTEX];
        float Weights[NUM_BONES_PER_VERTEX];
        void AddBoneData(unsigned int BoneID, float Weight)
        {
            for(unsigned int i = 0; i < NUM_BONES_PER_VERTEX; i++)
            {
                if(Weights[i] == 0.0f)
                {
                    IDs[i] = BoneID;
                    Weights[i] = Weight;
                    return;
                }
            }
        }
    };

    struct BoneInfo
    {
        bool isSkinned = false;
        glm::mat4 boneOffset;
        glm::mat4 defaultOffset;
        int parentIndex;
    };
    

    struct Model
    {
        MeshGroup Meshes;

        std::vector<BoneData> Bones;

        std::map<std::string, uint32_t> BoneMapping;
        uint32_t NumBones;
        std::vector<BoneInfo> BoneInfo;
        // TODO: Skeletal animation data structures
    };
} // namespace SnowLeopardEngine