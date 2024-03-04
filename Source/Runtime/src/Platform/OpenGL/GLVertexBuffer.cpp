#include "SnowLeopardEngine/Platform/OpenGL/GLVertexBuffer.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    GLVertexBuffer::GLVertexBuffer(uint32_t size)
    {
        glCreateBuffers(1, &m_BufferName);
        glNamedBufferStorage(m_BufferName, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    GLVertexBuffer::GLVertexBuffer(float* vertices, uint32_t size)
    {
        glCreateBuffers(1, &m_BufferName);
        glNamedBufferStorage(m_BufferName, size, vertices, GL_DYNAMIC_STORAGE_BIT);
    }

    GLVertexBuffer::GLVertexBuffer(std::vector<StaticMeshVertexData> vertices)
    {
        glCreateBuffers(1, &m_BufferName);
        glNamedBufferStorage(m_BufferName, vertices.size() * sizeof(StaticMeshVertexData), &vertices[0], GL_DYNAMIC_STORAGE_BIT);
    }

    GLVertexBuffer::GLVertexBuffer(std::vector<AnimatedMeshVertexData> vertices)
    {
        glCreateBuffers(1, &m_BufferName);
        glNamedBufferStorage(m_BufferName, vertices.size() * sizeof(AnimatedMeshVertexData), &vertices[0], GL_DYNAMIC_STORAGE_BIT);
    }

    GLVertexBuffer::~GLVertexBuffer() { glDeleteBuffers(1, &m_BufferName); }

    void GLVertexBuffer::SetBufferData(const void* data, uint32_t size)
    {
        glNamedBufferSubData(m_BufferName, 0, size, data);
    }
} // namespace SnowLeopardEngine