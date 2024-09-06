#pragma once

#include "SnowLeopardEngine/Function/Rendering/Buffer.h"

namespace SnowLeopardEngine
{
    class VertexBuffer final : public Buffer
    {
        friend class RenderContext;

    public:
        VertexBuffer() = default;

        GLsizei    GetStride() const;
        GLsizeiptr GetCapacity() const;

    private:
        VertexBuffer(Buffer, GLsizei stride);

    private:
        GLsizei m_Stride {0};
    };
} // namespace SnowLeopardEngine