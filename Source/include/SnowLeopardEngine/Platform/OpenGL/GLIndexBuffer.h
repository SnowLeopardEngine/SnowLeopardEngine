#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/IndexBuffer.h"

namespace SnowLeopardEngine
{
    class GLIndexBuffer : public IndexBuffer
    {
    public:
        GLIndexBuffer(uint32_t* indices, uint32_t count);
        explicit GLIndexBuffer(std::vector<uint32_t> indices);
        virtual ~GLIndexBuffer();

        virtual uint32_t GetCount() const { return m_Count; }

    private:
        uint32_t m_BufferName = 0;
        uint32_t m_Count      = 0;
    };
} // namespace SnowLeopardEngine