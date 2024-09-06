#pragma once

#include "SnowLeopardEngine/Function/Rendering/Buffer.h"

namespace SnowLeopardEngine
{
    enum class IndexType
    {
        Unknown = 0,
        UInt8   = 1,
        UInt16  = 2,
        UInt32  = 4
    };

    class IndexBuffer final : public Buffer
    {
        friend class RenderContext;

    public:
        IndexBuffer() = default;

        IndexType  GetIndexType() const;
        GLsizeiptr GetCapacity() const;

    private:
        IndexBuffer(Buffer, IndexType);

    private:
        IndexType m_IndexType {IndexType::Unknown};
    };
} // namespace SnowLeopardEngine