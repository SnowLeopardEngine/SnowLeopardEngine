#pragma once

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    class Buffer
    {
        friend class RenderContext;

    public:
        Buffer()              = default;
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) noexcept;
        virtual ~Buffer();

        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) noexcept;

        explicit operator bool() const;

        GLsizeiptr GetSize() const;
        bool       IsMapped() const;

    protected:
        Buffer(GLuint id, GLsizeiptr size);

        explicit operator GLuint() const;

    protected:
        GLuint     m_Id           = GL_NONE;
        GLsizeiptr m_Size         = 0;
        void*      m_MappedMemory = nullptr;
    };
} // namespace SnowLeopardEngine