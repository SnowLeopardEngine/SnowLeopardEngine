#include "SnowLeopardEngine/Platform/OpenGL/GLIndexBuffer.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    GLIndexBuffer::GLIndexBuffer(uint32_t* indices, uint32_t count) : m_Count(count)
    {
        glCreateBuffers(1, &m_BufferName);
        glNamedBufferStorage(m_BufferName, sizeof(uint32_t) * m_Count, indices, GL_DYNAMIC_STORAGE_BIT);
    }

    GLIndexBuffer::GLIndexBuffer(std::vector<uint32_t> indices) : m_Count(indices.size())
    {
        glCreateBuffers(1, &m_BufferName);
        glNamedBufferStorage(m_BufferName, sizeof(uint32_t) * m_Count, indices.data(), GL_DYNAMIC_STORAGE_BIT);
    }

    GLIndexBuffer::~GLIndexBuffer() { glDeleteBuffers(1, &m_BufferName); }
} // namespace SnowLeopardEngine