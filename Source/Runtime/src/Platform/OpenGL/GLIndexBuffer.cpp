#include "SnowLeopardEngine/Platform/OpenGL/GLIndexBuffer.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLAPI.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    GLIndexBuffer::GLIndexBuffer(uint32_t* indices, uint32_t count) : m_Count(count)
    {
        if (OpenGLAPI::IsDSASupported())
        {
            glCreateBuffers(1, &m_BufferName);
            glNamedBufferStorage(m_BufferName, sizeof(uint32_t) * m_Count, indices, GL_DYNAMIC_STORAGE_BIT);
        }
        else
        {
            glGenBuffers(1, &m_BufferName);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferName);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_Count, indices, GL_STATIC_DRAW);
        }
    }

    GLIndexBuffer::GLIndexBuffer(std::vector<uint32_t> indices) : m_Count(indices.size())
    {
        if (OpenGLAPI::IsDSASupported())
        {
            glCreateBuffers(1, &m_BufferName);
            glNamedBufferStorage(m_BufferName, sizeof(uint32_t) * m_Count, indices.data(), GL_DYNAMIC_STORAGE_BIT);
        }
        else
        {
            glGenBuffers(1, &m_BufferName);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferName);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_Count, indices.data(), GL_STATIC_DRAW);
        }
    }

    GLIndexBuffer::~GLIndexBuffer() { glDeleteBuffers(1, &m_BufferName); }

    void GLIndexBuffer::Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferName); }

    void GLIndexBuffer::Unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
} // namespace SnowLeopardEngine