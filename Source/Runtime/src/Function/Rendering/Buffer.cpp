#include "SnowLeopardEngine/Function/Rendering/Buffer.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include <cstring>

namespace SnowLeopardEngine
{
    Buffer::Buffer(Buffer&& other) noexcept :
        m_Id(other.m_Id), m_Size(other.m_Size), m_MappedMemory(other.m_MappedMemory)
    {
        memset(&other, 0, sizeof(Buffer));
    }

    Buffer::~Buffer()
    {
        if (m_Id != GL_NONE)
            SNOW_LEOPARD_CORE_ERROR("Buffer leak: {0}", m_Id);
    }

    Buffer& Buffer::operator=(Buffer&& rhs) noexcept
    {
        if (this != &rhs)
        {
            memcpy(this, &rhs, sizeof(Buffer));
            memset(&rhs, 0, sizeof(Buffer));
        }
        return *this;
    }

    Buffer::operator bool() const { return m_Id != GL_NONE; }

    GLsizeiptr Buffer::GetSize() const { return m_Size; }
    bool       Buffer::IsMapped() const { return m_MappedMemory != nullptr; }

    Buffer::Buffer(GLuint id, GLsizeiptr size) : m_Id(id), m_Size(size) {}

    Buffer::operator GLuint() const { return m_Id; }
} // namespace SnowLeopardEngine