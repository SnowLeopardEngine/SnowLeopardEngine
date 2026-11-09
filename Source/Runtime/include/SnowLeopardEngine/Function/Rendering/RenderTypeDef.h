#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Asset/OzzMesh.h"

namespace SnowLeopardEngine
{
    // clang-format off
    struct Offset2D
    {
        int32_t X {0}, Y {0};

        auto operator<=> (const Offset2D&) const = default;
    };

    struct Extent2D
    {
        uint32_t Width {0}, Height {0};

        auto operator<=> (const Extent2D&) const = default;
    };

    struct Rect2D
    {
        Offset2D Offset;
        Extent2D Extent;

        auto operator<=> (const Rect2D&) const = default;
    };
    // clang-format on

    using ViewportDesc = Rect2D;

    struct MeshVertexData
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;
    };
    static_assert(sizeof(MeshVertexData) == 32, "the renderer's vertex stream and cooked meshes (SAssetPipeline) use 32-byte vertices");

    struct MeshData
    {
        std::vector<MeshVertexData> Vertices;
        std::vector<uint32_t>       Indices;
    };

    struct MeshItem
    {
        std::string Name = "Untitled Mesh";
        MeshData    Data;
        bool        RenderLoaded = false;

        ozz::sample::Mesh OzzMesh;

        bool Skinned() const { return OzzMesh.skinned(); }
    };

    struct MeshGroup
    {
        std::vector<MeshItem> Items;
    };
} // namespace SnowLeopardEngine