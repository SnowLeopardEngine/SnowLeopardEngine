#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Asset/OzzMesh.h"
#include "SnowLeopardEngine/Function/Rendering/RenderHandle.h"

namespace SnowLeopardEngine
{
    class IndexBuffer;
    class VertexBuffer;
    class VertexFormat;

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

    enum class CompareOp : uint8_t
    {
        Never = 0,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };

    using ViewportDesc = Rect2D;

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

        Ref<IndexBuffer>  IdxBuffer  = nullptr;
        Ref<VertexBuffer> VertBuffer = nullptr;
        Ref<VertexFormat> VertFormat = nullptr;
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