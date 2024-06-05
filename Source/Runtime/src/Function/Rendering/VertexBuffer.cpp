#include "SnowLeopardEngine/Function/Rendering/VertexBuffer.h"
#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    GLsizei    VertexBuffer::GetStride() const { return m_Stride; }
    GLsizeiptr VertexBuffer::GetCapacity() const { return m_Size / m_Stride; }

    VertexBuffer::VertexBuffer(Buffer buffer, GLsizei stride) : Buffer {std::move(buffer)}, m_Stride {stride} {}
} // namespace SnowLeopardEngine