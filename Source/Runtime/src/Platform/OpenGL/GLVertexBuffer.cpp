#include "SnowLeopardEngine/Platform/OpenGL/GLVertexBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLAPI.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    GLVertexBuffer::GLVertexBuffer(uint32_t size)
    {
        if (OpenGLAPI::IsDSASupported())
        {
            glCreateBuffers(1, &m_BufferName);
            glNamedBufferStorage(m_BufferName, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
        }
        else
        {
            glGenBuffers(1, &m_BufferName);
            glBindBuffer(GL_ARRAY_BUFFER, m_BufferName);
            glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
        }
    }

    GLVertexBuffer::GLVertexBuffer(float* vertices, uint32_t size)
    {
        if (OpenGLAPI::IsDSASupported())
        {
            glCreateBuffers(1, &m_BufferName);
            glNamedBufferStorage(m_BufferName, size, vertices, GL_DYNAMIC_STORAGE_BIT);
        }
        else
        {
            glGenBuffers(1, &m_BufferName);
            glBindBuffer(GL_ARRAY_BUFFER, m_BufferName);
            glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
        }
    }

    GLVertexBuffer::GLVertexBuffer(const std::vector<MeshVertexData>& vertices)
    {
        if (OpenGLAPI::IsDSASupported())
        {
            glCreateBuffers(1, &m_BufferName);
            glNamedBufferStorage(
                m_BufferName, vertices.size() * sizeof(MeshVertexData), vertices.data(), GL_DYNAMIC_STORAGE_BIT);
        }
        else
        {
            glGenBuffers(1, &m_BufferName);
            glBindBuffer(GL_ARRAY_BUFFER, m_BufferName);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MeshVertexData), vertices.data(), GL_STATIC_DRAW);
        }
    }

    GLVertexBuffer::~GLVertexBuffer() { glDeleteBuffers(1, &m_BufferName); }

    void GLVertexBuffer::Bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_BufferName); }

    void GLVertexBuffer::Unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

    void GLVertexBuffer::SetBufferData(const void* data, uint32_t size)
    {
        if (OpenGLAPI::IsDSASupported())
        {
            glNamedBufferSubData(m_BufferName, 0, size, data);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_BufferName);
            glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
        }
    }

    void GLVertexBuffer::SetBufferData(const std::vector<MeshVertexData>& vertices)
    {
        if (OpenGLAPI::IsDSASupported())
        {
            glNamedBufferSubData(m_BufferName, 0, vertices.size() * sizeof(MeshVertexData), vertices.data());
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_BufferName);
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(MeshVertexData), vertices.data());
        }
    }
} // namespace SnowLeopardEngine