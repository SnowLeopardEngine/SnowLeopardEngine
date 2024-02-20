#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/VertexBuffer.h"

namespace SnowLeopardEngine
{
    class GLVertexBuffer : public VertexBuffer
    {
    public:
        explicit GLVertexBuffer(uint32_t size);
        GLVertexBuffer(float* vertices, uint32_t size);
        explicit GLVertexBuffer(std::vector<VertexData> vertices);

        virtual ~GLVertexBuffer();

        virtual void SetBufferData(const void* data, uint32_t size) override;

    private:
        uint32_t m_BufferName = 0;
    };
} // namespace SnowLeopardEngine