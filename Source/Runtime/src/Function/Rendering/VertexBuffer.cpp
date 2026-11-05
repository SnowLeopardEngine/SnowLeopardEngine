#include "SnowLeopardEngine/Function/Rendering/VertexBuffer.h"
#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    int32_t VertexBuffer::GetStride() const { return m_Stride; }
    int64_t VertexBuffer::GetCapacity() const { return m_Size / m_Stride; }

    VertexBuffer::VertexBuffer(Buffer buffer, int32_t stride) : Buffer {std::move(buffer)}, m_Stride {stride} {}
} // namespace SnowLeopardEngine