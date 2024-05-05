#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    struct VertexAttribute
    {
        enum class Type
        {
            Float = 0,
            Float2,
            Float3,
            Float4,

            Int,
            Int4,

            UByte4_Norm,
        };
        Type    VertType;
        int32_t Offset;
    };

    using VertexAttributes = std::map<int32_t, VertexAttribute>;

} // namespace SnowLeopardEngine

namespace std
{
    template<>
    struct hash<SnowLeopardEngine::VertexAttribute>
    {
        std::size_t operator()(const SnowLeopardEngine::VertexAttribute&) const noexcept;
    };
} // namespace std