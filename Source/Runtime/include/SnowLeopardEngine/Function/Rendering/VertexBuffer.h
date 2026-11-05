#pragma once

#include "SnowLeopardEngine/Function/Rendering/Buffer.h"

namespace SnowLeopardEngine
{
    class VertexBuffer final : public Buffer
    {
        friend class RenderContext;

    public:
        VertexBuffer() = default;

        int32_t GetStride() const;
        int64_t GetCapacity() const;

    private:
        VertexBuffer(Buffer, int32_t stride);

    private:
        int32_t m_Stride {0};
    };
} // namespace SnowLeopardEngine