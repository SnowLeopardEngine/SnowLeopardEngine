#include "SnowLeopardEngine/Function/Rendering/IndexBuffer.h"
#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    IndexType  IndexBuffer::GetIndexType() const { return m_IndexType; }
    GLsizeiptr IndexBuffer::GetCapacity() const { return m_Size / static_cast<GLsizei>(m_IndexType); }

    IndexBuffer::IndexBuffer(Buffer buffer, IndexType indexType) : Buffer {std::move(buffer)}, m_IndexType {indexType}
    {}
} // namespace SnowLeopardEngine